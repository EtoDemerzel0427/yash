//
// Created by Weiran Huang on 2/6/22.
//

#include "job.h"

Process *make_process(char *input) {
    if (input == NULL) {
//        fprintf(stderr, "make_process: No input provided.");
        return NULL;
    }
    Process *p = (Process *) malloc(sizeof(Process));

    p->next = NULL;
    p->argv = parse_process(input);
    p->pid = -2;  // will not be assigned until launch
    p->arg_in = p->arg_out = p->arg_err = NULL;
    p->status = Running;  // todo: update

    /* note that here we assume test cases are valid input
     * so we don't bother to perform any boundary checks for simplicity.
     */
    int in_pos = find_symbol_index(p->argv, "<");
    int out_pos = find_symbol_index(p->argv, ">");
    int err_pos = find_symbol_index(p->argv, "2>");
    if (in_pos != -1) {
        p->arg_in = p->argv[in_pos + 1];
        p->argv[in_pos] = NULL;
    }

    if (out_pos != -1) {
        p->arg_out = p->argv[out_pos + 1];
        p->argv[out_pos] = NULL;
    }

    if (err_pos != -1) {
        p->arg_err = p->argv[err_pos + 1];
        p->argv[err_pos] = NULL;
    }

    return p;
}

Job *make_job(char *input) {
    printf("input: %s input len: %lu\n", input, strlen(input));
    if (input == NULL) {
        fprintf(stderr, "make_job: no input provided\n");
        return NULL;
    }

    Job *j = (Job *) malloc(sizeof(Job));
    if (j == NULL) {
        fprintf(stderr, "malloc error");
        exit(EXIT_FAILURE);
    }

    j->next = NULL;
    j->command = (char *) malloc(MAX_LINE_LEN + 1);
    strcpy(j->command, input);
    j->background = false;
    char *lptr, *rptr;
    if (input[strlen(input) - 1] == '&') {
        input[strlen(input) - 1] = '\0';
        j->background = true;
    }
    parse_pipe(input, &lptr, &rptr);

    if (lptr != NULL && rptr != NULL) {
        j->left = make_process(lptr);
        j->right = make_process(rptr);
        j->left->next = j->right;  // todo: remove next
    } else if (rptr == NULL) {
        j->left = make_process(lptr);
        j->right = NULL;
    }

    return j;
}

void release_process(Process *p) {
    free(p->argv);
    free(p);
}

void release_job(Job *j) {
    free(j->command);
    if (j->left)
        release_process(j->left);
    if (j->right)
        release_process(j->right);
    free(j);
}