/*
 * Created by Weiran Huang on 2/6/22.
 * The design of this part are borrowed from gnu.org.
 *
 */




#ifndef YASH_JOB_H
#define YASH_JOB_H
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <termios.h>
#include "parse.h"

#define MAX_JOBS 25

/* shell related */
pid_t shell_pgid;
int shell_terminal;


typedef enum {
    Running,
    Stopped,
    Completed
} stat_t;


typedef struct proc_t
{
    struct proc_t *next;                              /* next process in pipeline */
    char **argv;                                      /* for exec */
    char *arg_in, *arg_out, *arg_err;                 /* the args after > < 2> respectively */
    pid_t pid;                                        /* process ID */
    stat_t stat;
} proc_t;


typedef struct job_t
{
    struct job_t *next;
    char *command;
    proc_t *left;
    proc_t *right;                 /* as there is at most one pipe, we can simplify this as left right processes */
    pid_t pgid;                    /* process group ID */
    int job_id;                    /* the index for `jobs`, and the background job terminated message */
    bool notified;                 /* true if user told about stopped job */
    bool background;               /* indication of whether this job is running in foreground or background */
} job_t;

extern job_t *cur_job;

proc_t *make_process(char *input);
job_t *make_job(char *input);
void release_process(proc_t *);
void release_job(job_t *);
void launch_process(proc_t * p, int pipe_in, int pipe_out);
void launch_job(job_t *);
int mark_child_status_on_signal(pid_t pid, int status);
bool job_is_completed(job_t *j);
bool job_is_stopped(job_t *j);
void notify_background_job();
void print_all_jobs();
void fg();
void bg();

#endif //YASH_JOB_H
