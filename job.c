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
        j->right->next = NULL;
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

void launch_process(Process *p, int pipe_in, int pipe_out) {
    int infile = open(p->arg_in, O_RDONLY);
    int outfile = open(p->arg_out, O_WRONLY | O_CREAT);
    int errfile = open(p->arg_err, O_WRONLY | O_CREAT);

    /* redirection */
    if (infile >= 0 && infile != STDIN_FILENO) {
        dup2(infile, STDIN_FILENO);
        close(infile);
    }

    if (outfile >= 0 && outfile != STDOUT_FILENO) {
        dup2(outfile, STDOUT_FILENO);
        close(outfile);
    }

    if (errfile >= 0 && errfile != STDERR_FILENO) {
        dup2(errfile, STDERR_FILENO);
        close(errfile);
    }

    /* piping */
    if (pipe_in != STDIN_FILENO) {
        printf("pipe_in: %d\n", pipe_in);
        dup2(pipe_in, STDIN_FILENO);
        close(pipe_in);
    }

    if (pipe_out != STDOUT_FILENO) {
        printf("pipe_out: %d\n", pipe_out);
        dup2(pipe_out, STDOUT_FILENO);
        close(pipe_out);
    }

    if (execvp(p->argv[0], p->argv) == -1)
        perror("launch process error.");

    exit(EXIT_FAILURE);
}

void launch_job(Job *j) {
    int mypipe[2];
    pid_t pid;
    int infile = STDIN_FILENO, outfile = STDOUT_FILENO;

    Process *p = j->left;
    while (p != NULL) {
        if (p->next) {
            /* there is a pipe */
            if (pipe(mypipe) < 0) {
                perror("pipe error");
                exit(EXIT_FAILURE);
            }

            outfile = mypipe[1];
        } else {
            outfile = STDOUT_FILENO;
        }

        pid = fork();
        if (pid == 0) {
            /* child */
            printf("infile: %d\n", infile);
            printf("outfile: %d\n", outfile);
            launch_process(p, infile, outfile);
        } else if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else {
            /* parent */
            int wpid, status;
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }

        if (infile != STDIN_FILENO)
            close(infile);
        if (outfile != STDOUT_FILENO)
            close(outfile);
        infile = mypipe[0];

        p = p->next;
    }

}