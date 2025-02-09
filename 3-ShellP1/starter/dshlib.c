#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
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

        char *subsaveptr;
        char *token = strtok_r(segment, " ", &subsaveptr);
        if (token == NULL) {
            segment = strtok_r(NULL, PIPE_STRING, &saveptr);
            continue;
        }
        strncpy(clist->commands[count].exe, token, EXE_MAX - 1);
        clist->commands[count].exe[EXE_MAX - 1] = '\0';

        char args_buffer[ARG_MAX] = "";
        int first_arg = 1;
        while ((token = strtok_r(NULL, " ", &subsaveptr)) != NULL) {
            if (first_arg) {
                first_arg = 0;
                snprintf(args_buffer, ARG_MAX, "%s", token);
            } else {
                strncat(args_buffer, " ", ARG_MAX - strlen(args_buffer) - 1);
                strncat(args_buffer, token, ARG_MAX - strlen(args_buffer) - 1);
            }
        }
        strncpy(clist->commands[count].args, args_buffer, ARG_MAX - 1);
        clist->commands[count].args[ARG_MAX - 1] = '\0';

        count++;
        segment = strtok_r(NULL, PIPE_STRING, &saveptr);
    }

    if (count == 0) {
        return WARN_NO_CMDS;
    }

    clist->num = count;
    return OK;
}