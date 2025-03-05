#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"

/* Added definitions to satisfy missing symbols */
#ifndef BI_RC
#define BI_RC 5
#endif

#ifndef CMD_ERR_EXECUTE
#define CMD_ERR_EXECUTE "error: command execution failed"
#endif

// Helper functions for cmd_buff_t management
int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    cmd_buff->_cmd_buffer = NULL;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    for (int i = 0; i < cmd_buff->argc; i++) {
        free(cmd_buff->argv[i]);
        cmd_buff->argv[i] = NULL;
    }
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    free_cmd_buff(cmd_buff);
    return alloc_cmd_buff(cmd_buff);
}

/*
 * Splits the command line into tokens.
 * It trims spaces and treats quoted strings as a single token.
 */
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    clear_cmd_buff(cmd_buff);
    char *p = cmd_line;
    while (*p != '\0') {
        while (*p == SPACE_CHAR) p++;
        if (*p == '\0') break;
        char *token = NULL;
        if (*p == '"') {
            p++;
            token = p;
            char *end_quote = strchr(p, '"');
            if (end_quote == NULL) {
                fprintf(stderr, "Unbalanced quotes in command line\n");
                return ERR_CMD_ARGS_BAD;
            }
            *end_quote = '\0';
            p = end_quote + 1;
        } else {
            token = p;
            while (*p != SPACE_CHAR && *p != '\0') p++;
            if (*p != '\0') {
                *p = '\0';
                p++;
            }
        }
        if (cmd_buff->argc < CMD_ARGV_MAX - 1) {
            cmd_buff->argv[cmd_buff->argc] = strdup(token);
            if (cmd_buff->argv[cmd_buff->argc] == NULL) {
                perror("strdup");
                return ERR_MEMORY;
            }
            cmd_buff->argc++;
        } else {
            fprintf(stderr, "Too many arguments provided\n");
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
    }
    cmd_buff->argv[cmd_buff->argc] = NULL;
    if (cmd_buff->argc == 0)
        return WARN_NO_CMDS;
    return OK;
}

/*
 * build_cmd_list
 *  Splits the input command line into separate commands based on the pipe (|)
 *  and stores each command into a command_list_t structure.
 *  It trims leading and trailing spaces from each command segment.
 */
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    clist->num = 0;
    char *p = cmd_line;
    while (*p != '\0' && *p == ' ') {
        p++;
    }
    if (*p == '\0') {
        return WARN_NO_CMDS;
    }
    int count = 0;
    char *saveptr;
    char *segment = strtok_r(cmd_line, PIPE_STRING, &saveptr);
    while (segment != NULL) {
        while (*segment == ' ') {
            segment++;
        }
        int len = strlen(segment);
        while (len > 0 && segment[len - 1] == ' ') {
            segment[len - 1] = '\0';
            len--;
        }
        if (strlen(segment) == 0) {
            segment = strtok_r(NULL, PIPE_STRING, &saveptr);
            continue;
        }
        if (count >= CMD_MAX) {
            return ERR_TOO_MANY_COMMANDS;
        }
        alloc_cmd_buff(&clist->commands[count]);
        clist->commands[count]._cmd_buffer = strdup(segment);
        if (clist->commands[count]._cmd_buffer == NULL) {
            return ERR_MEMORY;
        }
        int rc = build_cmd_buff(segment, &clist->commands[count]);
        if (rc != OK) {
            return rc;
        }
        count++;
        segment = strtok_r(NULL, PIPE_STRING, &saveptr);
    }
    if (count == 0) {
        return WARN_NO_CMDS;
    }
    clist->num = count;
    return OK;
}

int free_cmd_list(command_list_t *cmd_lst) {
    for (int i = 0; i < cmd_lst->num; i++) {
        free_cmd_buff(&cmd_lst->commands[i]);
    }
    cmd_lst->num = 0;
    return OK;
}

// Check if the command is a built-in command.
Built_In_Cmds match_command(const char *input) {
    if (strcmp(input, EXIT_CMD) == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "rc") == 0)
        return BI_RC;
    return BI_NOT_BI;
}

// Execute built-in commands.
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds bicmd = match_command(cmd->argv[0]);
    if (bicmd == BI_CMD_CD) {
        if (cmd->argc < 2) {
            return BI_EXECUTED;
        } else {
            if (chdir(cmd->argv[1]) != 0) {
                perror("chdir");
            }
            return BI_EXECUTED;
        }
    } else if (bicmd == BI_CMD_EXIT) {
        exit(0);
    } else if (bicmd == BI_RC) {
        // Extra credit: rc command placeholder.
        printf("rc not implemented\n");
        return BI_EXECUTED;
    }
    return BI_NOT_BI;
}

// Execute external commands using fork/execvp.
int exec_cmd(cmd_buff_t *cmd) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return ERR_EXEC_CMD;
    }
    if (pid == 0) {
        execvp(cmd->argv[0], cmd->argv);
        perror("execvp");
        exit(ERR_EXEC_CMD);
    } else {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
int exec_local_cmd_loop()
{
    char input_line[SH_CMD_MAX + 1];
    int rc = 0;
    command_list_t cmd_list;

    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(input_line, sizeof(input_line), stdin) == NULL) {
            printf("\n");
            break;
        }
        input_line[strcspn(input_line, "\n")] = '\0';

        if (strlen(input_line) == 0) {
            printf("%s", CMD_WARN_NO_CMD);
            continue;
        }

        if (strcmp(input_line, EXIT_CMD) == 0) {
            printf("exiting...\n");
            break;
        }

        rc = build_cmd_list(input_line, &cmd_list);
        if (rc != OK) {
            if (rc == WARN_NO_CMDS)
                printf("%s", CMD_WARN_NO_CMD);
            else if (rc == ERR_TOO_MANY_COMMANDS)
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        }

        rc = execute_pipeline(&cmd_list);
        if (rc != OK) {
            printf("%s\n", CMD_ERR_EXECUTE);
        }

        free_cmd_list(&cmd_list);
    }
    return OK;
}

/*
 * Executes a pipeline of commands. For each command, a child is forked.
 * Pipes are set up between children using dup2. The parent waits for all.
 */
int execute_pipeline(command_list_t *clist) {
    int num_cmds = clist->num;
    int pipefd[2 * (num_cmds - 1)];
    pid_t pids[CMD_MAX];

    // Create pipes between commands
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipefd + i * 2) < 0) {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }

    // Fork for each command in the pipeline
    for (int i = 0; i < num_cmds; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            return ERR_EXEC_CMD;
        }
        if (pids[i] == 0) {  // Child process
            // If not the first command, set STDIN to read end of previous pipe
            if (i > 0) {
                if (dup2(pipefd[(i - 1) * 2], STDIN_FILENO) < 0) {
                    perror("dup2");
                    exit(ERR_EXEC_CMD);
                }
            }
            // If not the last command, set STDOUT to write end of current pipe
            if (i < num_cmds - 1) {
                if (dup2(pipefd[i * 2 + 1], STDOUT_FILENO) < 0) {
                    perror("dup2");
                    exit(ERR_EXEC_CMD);
                }
            }

            // Close all pipe file descriptors in child
            for (int j = 0; j < 2 * (num_cmds - 1); j++) {
                close(pipefd[j]);
            }

            // Execute the command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(ERR_EXEC_CMD);
        }
    }

    // Parent: Close all pipe file descriptors
    for (int i = 0; i < 2 * (num_cmds - 1); i++) {
        close(pipefd[i]);
    }

    // Parent waits for all child processes
    for (int i = 0; i < num_cmds; i++) {
        waitpid(pids[i], NULL, 0);
    }

    return OK;
}
