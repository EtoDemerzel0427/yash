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

pid_t shell_pgid;
struct termios shell_tmodes;
int shell_terminal;
int shell_is_interactive;

typedef enum {
    Running,
    Stopped,
    Completed
} stat_t;

/* A process is a single process.  */
typedef struct proc_t
{
    struct proc_t *next;                             /* next process in pipeline */
    char **argv;                                      /* for exec */
    char *arg_in, *arg_out, *arg_err;                 /* the args after > < 2> respectively */
    pid_t pid;                                        /* process ID */
    stat_t stat;                                       /* reported stat value */
} proc_t;

/* A job is a pipeline of processes.  */
typedef struct job_t
{
    struct job_t *next;           /* next active job */
    char *command;              /* command line, used for messages */
    proc_t *left;
    proc_t *right;             /* as there is at most one pipe, we can simplify this as left right processes */
    pid_t pgid;                 /* process group ID */
    stat_t stat;
//    char notified;              /* true if user told about stopped job */
//    struct termios tmodes;      /* saved terminal modes */
    bool background;            /* indication of whether this job is running in foreground or background */
//    int stdin, stdout, stderr;  /* standard i/o channels */
} job_t;

proc_t *make_process(char *input);
job_t *make_job(char *input);
void release_process(proc_t *);
void release_job(job_t *);
void launch_process(proc_t * p, int pipe_in, int pipe_out);
void launch_job(job_t *);
int mark_child_status_on_signal(pid_t pid, int status);
bool job_is_completed(job_t *j);
bool job_is_stopped(job_t *j);

#endif //YASH_JOB_H
