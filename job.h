//
// Created by Weiran Huang on 2/6/22.
//

#ifndef YASH_JOB_H
#define YASH_JOB_H
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "parse.h"

enum {
    Running,
    Stopped,
    Done
};

/* A process is a single process.  */
typedef struct Process
{
    struct Process *next;                             /* next process in pipeline */
    char **argv;                                      /* for exec */
    char *arg_in, *arg_out, *arg_err;                 /* the args after > < 2> respectively */
    pid_t pid;                                        /* process ID */
    int status;                                       /* reported status value */
} Process;

/* A job is a pipeline of processes.  */
typedef struct Job
{
    struct Job *next;           /* next active job */
    char *command;              /* command line, used for messages */
    Process *left;
    Process *right;             /* as there is at most one pipe, we can simplify this as left right processes */
    pid_t pgid;                 /* process group ID */
//    char notified;              /* true if user told about stopped job */
//    struct termios tmodes;      /* saved terminal modes */
    bool background;            /* indication of whether this job is running in foreground or background */
//    int stdin, stdout, stderr;  /* standard i/o channels */
} Job;

Process *make_process(char *input);
Job *make_job(char *input);
void release_process(Process *);
void release_job(Job *);
void launch_process(Process * p, int pipe_in, int pipe_out);
void launch_job(Job *);

#endif //YASH_JOB_H
