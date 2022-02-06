#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "parse.h"
#include "job.h"

int main() {
    char *user_input;
    while ((user_input = readline("# ")) != NULL) {
        if (strlen(user_input) > 0) {
            add_history(user_input);
        }

        user_input = trim_space(user_input);
        if (user_input[strlen(user_input - 1)] == '\n') {
            user_input[strlen(user_input) - 1] = '\0';
        }

        Job *j = make_job(user_input);

        printf("command:%s\n", j->command);
        printf("background: %d\n", j->background);
        if (j->left) {
            for (int i = 0; i < MAX_TOEKN_NUM; i++) {
                printf("token: %s\n", j->left->argv[i]);
            }
            printf("arg_in: %s\n", j->left->arg_in);
            printf("arg_out: %s\n", j->left->arg_out);
            printf("arg_err: %s\n", j->left->arg_err);
        }

        if (j->right) {
            for (int i = 0; i < MAX_TOEKN_NUM; i++) {
                printf("token: %s\n", j->right->argv[i]);
            }
            printf("arg_in: %s\n", j->right->arg_in);
            printf("arg_out: %s\n", j->right->arg_out);
            printf("arg_err: %s\n", j->right->arg_err);
        }

        release_job(j);
    }

    return 0;
}
