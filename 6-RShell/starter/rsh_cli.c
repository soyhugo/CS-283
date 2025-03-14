#include <sys/socket.h>
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
 * exec_remote_cmd_loop(server_ip, port)
 */
int exec_remote_cmd_loop(char *address, int port) {
    int client_sock = start_client(address, port);
    if (client_sock < 0) {
        return ERR_RDSH_CLIENT;
    }

    char cmd_buffer[SH_CMD_MAX];
    char response_buffer[RDSH_COMM_BUFF_SZ];

    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buffer, sizeof(cmd_buffer), stdin) == NULL) {
            printf("\n");
            break;
        }

        cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0';

        if (strcmp(cmd_buffer, "exit") == 0) {
            printf("Exiting client...\n");
            break;
        }

        send(client_sock, cmd_buffer, strlen(cmd_buffer) + 1, 0);

        ssize_t bytes_received;
        while ((bytes_received = recv(client_sock, response_buffer, RDSH_COMM_BUFF_SZ, 0)) > 0) {
            response_buffer[bytes_received] = '\0';
            if (response_buffer[bytes_received - 1] == RDSH_EOF_CHAR) {
                response_buffer[bytes_received - 1] = '\0'; 
                printf("%s\n", response_buffer);
                break;
            }
            printf("%s", response_buffer);
        }
    }

    close(client_sock);
    return OK;
}


/*
 * start_client(server_ip, port)
 */
int start_client(char *server_ip, int port) {
    int client_sock;
    struct sockaddr_in server_addr;

    // Create socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == -1) {
        perror("socket");
        return ERR_RDSH_CLIENT;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(port);

    // Connect to the server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_sock);
        return ERR_RDSH_CLIENT;
    }

    return client_sock;
}


/*
 * client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc)
 */
int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc)
{
    if (cli_socket > 0)
    {
        close(cli_socket);
    }

    free(cmd_buff);
    free(rsp_buff);

    return rc;
}
