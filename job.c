//
// Created by Weiran Huang on 2/6/22.
//

#include "job.h"
#include <errno.h>

extern job_t *cur_job;

proc_t *make_process(char *input) {
    if (input == NULL) {
//        fprintf(stderr, "make_process: No input provided.");
        return NULL;
    }
    proc_t *p = (proc_t *) malloc(sizeof(proc_t));

    p->next = NULL;
    p->argv = parse_process(input);
    p->pid = -2;  // will not be assigned until launch
    p->arg_in = p->arg_out = p->arg_err = NULL;
    p->stat = Running;  // todo: update

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

job_t *make_job(char *input) {
    if (input == NULL || strlen(input) == 0) {
        fprintf(stderr, "make_job: no input provided\n");
        return NULL;
    }

    job_t *j = (job_t *) malloc(sizeof(job_t));
    if (j == NULL) {
        fprintf(stderr, "malloc error");
        exit(EXIT_FAILURE);
    }

    j->pgid = -2;
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
        j->left->next = j->right;
        j->right->next = NULL;
    } else if (rptr == NULL) {
        j->left = make_process(lptr);
        j->right = NULL;
    }

    return j;
}

void release_process(proc_t *p) {
    free(p->argv);
    free(p);
}

void release_job(job_t *j) {
    free(j->command);
    if (j->left)
        release_process(j->left);
    if (j->right)
        release_process(j->right);
    free(j);
}

void launch_process(proc_t *p, int pipe_in, int pipe_out) {
    fprintf(stderr, "process pid: %d, pgid: %d\n", getpid(), getpgid(getpid()));
    signal (SIGINT, SIG_DFL);
    signal (SIGQUIT, SIG_DFL);
    signal (SIGTSTP, SIG_DFL);
    signal (SIGTTIN, SIG_DFL);
    signal (SIGTTOU, SIG_DFL);
    signal (SIGCHLD, SIG_DFL);

    int infile = open(p->arg_in, O_RDONLY);
    int outfile = open(p->arg_out, O_WRONLY | O_CREAT | O_TRUNC);
    int errfile = open(p->arg_err, O_WRONLY | O_CREAT | O_TRUNC);

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
        dup2(pipe_in, STDIN_FILENO);
        close(pipe_in);
    }

    if (pipe_out != STDOUT_FILENO) {
        dup2(pipe_out, STDOUT_FILENO);
        close(pipe_out);
    }

    if (execvp(p->argv[0], p->argv) == -1)
        perror("launch process error.");

    exit(EXIT_FAILURE);
}

void launch_job(job_t *j) {
    int mypipe[2];
    pid_t pid, pgid = j->pgid;
    int infile = STDIN_FILENO, outfile = STDOUT_FILENO;

    proc_t *p = j->left;
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
            p->pid = getpid();
            if (pgid < 0)
                pgid = p->pid;
            setpgid(p->pid, pgid);
            if (!j->background) {
                tcsetpgrp(shell_terminal, pgid);
            }
            launch_process(p, infile, outfile);
        } else if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else {
            /* parent. Repeat to avoid race condition. */
            p->pid = pid;
            if (pgid < 0)
                pgid = p->pid;
            setpgid(p->pid, pgid);
            if (!j->background) {
                tcsetpgrp(shell_terminal, pgid);
            }
//            int wpid, stat;
//            do {
//                wpid = waitpid(WAIT_ANY, &stat, WUNTRACED);
//            } while (!WIFEXITED(stat) && !WIFSIGNALED(stat));
        }

        if (infile != STDIN_FILENO)
            close(infile);
        if (outfile != STDOUT_FILENO)
            close(outfile);
        infile = mypipe[0];

        p = p->next;
    }
    if (!j->background) {
        int wpid, status;
        do {
            wpid = waitpid(WAIT_ANY, &status, WUNTRACED);
            printf("wpid: %d\n", wpid);
        } while (!mark_child_status_on_signal(wpid, status) && !job_is_completed(j) && !job_is_stopped(j));

        if (j->left)
            printf("left stat: %d\n", j->left->stat);
        if (j->right)
            printf("right stat: %d\n", j->right->stat);
        printf("errno: %d\n", errno);
    }

    tcsetpgrp(shell_terminal, shell_pgid);
}

/* when waitpid returns, update the status of the corresponding process
 * pid is the returned value of waitpid.
 * */
int mark_child_status_on_signal(pid_t pid, int status) {
    if (pid < 0) {
        perror("waitpid error.\n");
        return -1;
    }

    if (pid == 0 || errno == ECHILD) {
        return -1;
    }
    for (job_t *j = cur_job; j != NULL; j = j->next) {
        for (proc_t *p = j->left; p != NULL; p = p->next) {
            if (p->pid == pid) {
                if (WIFSTOPPED(status)) {
                    /* stopped by SIGTSTP */
                    p->stat = Stopped;
                } else {
                    p->stat = Completed;   // Terminated by ctrl-z is also a form of completion
                    if (WIFSIGNALED (status))
                        fprintf (stderr, "%d: Terminated by signal %d.\n",
                                 (int) pid, WTERMSIG (status));
                }
                return 0; // so that continue to wait
            }
        }
    }
    fprintf(stderr, "No such child process: %d\n", (int) pid);
    return -1;
}

bool job_is_completed(job_t *j) {
    /* using this for loop to easily expand to multi-pipe */
    for (proc_t *p = j->left; p != NULL; p = p->next) {
        if (p->stat != Completed)
            return false;
    }
    return true;
}

bool job_is_stopped(job_t *j) {
    if (job_is_completed(j)) return false;

    for (proc_t *p = j->left; p != NULL; p = p->next) {
        if (p->stat != Stopped && p->stat != Completed) {
            return false;
        }
    }

    return true;
}
