#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
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

        launch_job(j);

        release_job(j);
    }

    return 0;
}
