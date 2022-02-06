//
// Created by Weiran Huang on 2/6/22.
//

#ifndef YASH_PARSE_H
#define YASH_PARSE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_TOEKN_NUM 30
#define MAX_TOKEN_LEN 30
#define MAX_LINE_LEN 2000

char *trim_space(char *str);
void parse_pipe(char *input, char **lptr, char **rptr);
char** parse_process(char *cmd);
int find_symbol_index(char **args, char *symbol);

#endif //YASH_PARSE_H
