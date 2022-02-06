//
// Created by Weiran Huang on 2/6/22.
//

#include "parse.h"

char *trim_space(char *str)
{
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

void parse_pipe(char *input, char **lptr, char **rptr) {
    *lptr = strtok(input, "|");
    *rptr = strtok(NULL, "|");
    if (strtok(NULL, "|") != NULL) {
        fprintf(stderr, "We do not support more than one pipe.");
        exit(EXIT_FAILURE);
    }
}

char** parse_process(char *input) {
    if (input == NULL) {
        fprintf(stderr, "No command provided\n");
        return NULL;
    }
    char **args = (char **) malloc(MAX_TOEKN_NUM * sizeof(char *));
    memset(args, 0, sizeof(char *) * MAX_TOEKN_NUM);
    char *token = strtok(input, "\t\n ");
    int i = 0;
    while (token != NULL) {
//        printf("token: %s\n", token);
        args[i++] = token;
        token = strtok(NULL, "\t\n ");
    }
    args[i] = NULL; //"\0";

    return args;
}

int find_symbol_index(char **args, char *symbol) {
    for (int i = 0; i < MAX_TOEKN_NUM && args[i] != NULL; i++) {
        if (strcmp(args[i], symbol) == 0) {
            return i;
        }
    }

    return -1;
}
