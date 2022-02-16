#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <termio.h>
#include "parse.h"
#include "job.h"

job_t *cur_job = NULL;

void init_shell() {
    shell_terminal = STDIN_FILENO;


    /* the shell will ignore these signals itself, but let jobs running in it handle them */
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, SIG_DFL);

    shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) < 0) {
        perror("Couldn't put the shell in its own process group");
        exit(EXIT_FAILURE);
    }

    tcsetpgrp(shell_terminal, shell_pgid);

//    tcgetattr(shell_terminal, &shell_tmodes);
    // Ensure that three file descriptors are open.
    int fd;
    while((fd = open("console", O_RDWR)) >= 0){
        if(fd >= 3){
            close(fd);
            break;
        }
    }

}

int main() {
    init_shell();
    char *user_input;
    while ((user_input = readline("# ")) != NULL) {
        notify_background_job();
        if (strlen(user_input) > 0) {
            add_history(user_input);
        }

        user_input = trim_space(user_input);
        if (user_input[strlen(user_input - 1)] == '\n') {
            user_input[strlen(user_input) - 1] = '\0';
        }

        if (strlen(user_input) == 0)
            continue;

        if (strchr(user_input, '&') != NULL && strchr(user_input, '&') - user_input < strlen(user_input) - 1) {
            printf("& must be the last char.\n");
            continue;
        }


        /*********************    builtins    *******************/
        if (strcmp(user_input, "jobs") == 0) {
            print_all_jobs();
            continue;
        }

        if (strcmp(user_input, "fg") == 0) {
            fg();
            continue;
        }

        if (strcmp(user_input, "bg") == 0) {
            bg();
            continue;
        }

        job_t *j = make_job(user_input);
        if (j == NULL)
            continue;
        j->next = cur_job;
        cur_job = j;

        launch_job(j);

//        release_job(j);
    }

    return 0;
}
