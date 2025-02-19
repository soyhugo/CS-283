#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

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
            p++; // skip opening quote
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
    cmd_buff_t cmd;

    alloc_cmd_buff(&cmd);

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

        cmd._cmd_buffer = strdup(input_line);
        if (cmd._cmd_buffer == NULL) {
            perror("strdup");
            continue;
        }

        rc = build_cmd_buff(input_line, &cmd);
        if (rc != OK) {
            if (rc == WARN_NO_CMDS)
                printf("%s", CMD_WARN_NO_CMD);
            free_cmd_buff(&cmd);
            continue;
        }

        Built_In_Cmds bicmd = match_command(cmd.argv[0]);
        if (bicmd != BI_NOT_BI) {
            exec_built_in_cmd(&cmd);
        } else {
            rc = exec_cmd(&cmd);
            if (rc == ERR_EXEC_CMD) {
                printf("%s\n", "CMD_ERR_EXECUTE");
            }
        }
        free_cmd_buff(&cmd);
    }
    return OK;
}
