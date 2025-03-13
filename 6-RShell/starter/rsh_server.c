#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include "dshlib.h"
#include "rshlib.h"

/*
 * start_server(ifaces, port, is_threaded)
 */
int start_server(char *ifaces, int port, int is_threaded)
{
    (void)is_threaded;
    int svr_socket;
    int rc;

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0)
    {
        return svr_socket;
    }

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);

    return rc;
}

/*
 * stop_server(svr_socket)
 */
int stop_server(int svr_socket)
{
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 */
int boot_server(char *ifaces, int port) {
    int server_sock;
    struct sockaddr_in server_addr;
    int opt = 1;

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    // Allow port reuse
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ifaces);
    server_addr.sin_port = htons(port);

    // Bind socket
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_sock);
        return ERR_RDSH_COMMUNICATION;
    }

    // Start listening
    if (listen(server_sock, 5) == -1) {
        perror("listen");
        close(server_sock);
        return ERR_RDSH_COMMUNICATION;
    }

    printf("Server listening on %s:%d\n", ifaces, port);
    return server_sock;
}


/*
 * process_cli_requests(svr_socket)
 */
int process_cli_requests(int svr_socket) {
    int client_sock;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        client_sock = accept(svr_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }

        printf("Client connected!\n");

        int rc = exec_client_requests(client_sock);
        if (rc == OK_EXIT) {
            printf("Server stopping as requested by client.\n");
            break;
        }

        close(client_sock);
    }
    return OK;
}

/*
 * exec_client_requests(int cli_socket)
 */
int exec_client_requests(int cli_socket) {
    char buffer[SH_CMD_MAX];

    while (1) {
        ssize_t bytes_received = recv(cli_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        buffer[bytes_received] = '\0';  // Null-terminate the command
        printf("Executing command: %s\n", buffer);  // Debugging output

        // Handle built-in commands
        if (strcmp(buffer, "stop-server") == 0) {
            close(cli_socket);
            return OK_EXIT;
        } else if (strncmp(buffer, "cd ", 3) == 0) {
            char *dir = buffer + 3;
            if (chdir(dir) == 0) {
                send_message_string(cli_socket, "Directory changed.");
            } else {
                send_message_string(cli_socket, "Failed to change directory.");
            }
            continue;
        }

        command_list_t cmd_list;
        if (build_cmd_list(buffer, &cmd_list) != OK) {
            send_message_string(cli_socket, "Invalid command.");
            continue;
        }

        // Fork to execute command
        pid_t pid = fork();
        if (pid == 0) {  // Child process
            // Redirect stdout and stderr to the client socket
            dup2(cli_socket, STDOUT_FILENO);
            dup2(cli_socket, STDERR_FILENO);

            // Close unused descriptors
            close(cli_socket);

            // Execute the command pipeline
            execute_pipeline(&cmd_list);

            // Flush stdout and stderr to ensure all output is sent to the client
            fflush(stdout);
            fflush(stderr);

            exit(0);  // Exit child process after execution
        } 
        else if (pid > 0) {  // Parent process
            waitpid(pid, NULL, 0);
        } 
        else {  // Fork failed
            perror("fork");
            send_message_string(cli_socket, "Failed to execute command.");
        }

        free_cmd_list(&cmd_list);
        send_message_eof(cli_socket);  // Signal end of response
    }
    return OK;
}

/*
 * send_message_eof(cli_socket)
 */
int send_message_eof(int cli_socket)
{
    if (send(cli_socket, &RDSH_EOF_CHAR, 1, 0) < 0)
    {
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

/*
 * send_message_string(cli_socket, char *buff)
 */
int send_message_string(int cli_socket, char *buff)
{
    int len = strlen(buff) + 1; // +1 to include null terminator
    if (send(cli_socket, buff, len, 0) < 0)
    {
        return ERR_RDSH_COMMUNICATION;
    }

    return send_message_eof(cli_socket);
}

/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist)
{
    int num_cmds = clist->num;
    int pipefd[2 * (num_cmds - 1)];
    pid_t pids[CMD_MAX];

    // Create pipes
    for (int i = 0; i < num_cmds - 1; i++)
    {
        if (pipe(pipefd + i * 2) < 0)
        {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }

    // Fork processes for each command
    for (int i = 0; i < num_cmds; i++)
    {
        pids[i] = fork();
        if (pids[i] < 0)
        {
            perror("fork");
            return ERR_EXEC_CMD;
        }
        if (pids[i] == 0)
        {
            // Redirect stdin for the first command
            if (i == 0)
            {
                dup2(cli_sock, STDIN_FILENO);
            }
            else
            {
                dup2(pipefd[(i - 1) * 2], STDIN_FILENO);
            }

            // Redirect stdout for the last command
            if (i == num_cmds - 1)
            {
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            }
            else
            {
                dup2(pipefd[i * 2 + 1], STDOUT_FILENO);
            }

            // Close all pipes in the child
            for (int j = 0; j < 2 * (num_cmds - 1); j++)
            {
                close(pipefd[j]);
            }

            // Execute the command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(ERR_EXEC_CMD);
        }
    }

    // Close all pipe file descriptors in the parent
    for (int i = 0; i < 2 * (num_cmds - 1); i++)
    {
        close(pipefd[i]);
    }

    // Wait for all child processes
    int status;
    for (int i = 0; i < num_cmds; i++)
    {
        waitpid(pids[i], &status, 0);
    }

    return WEXITSTATUS(status);
}
