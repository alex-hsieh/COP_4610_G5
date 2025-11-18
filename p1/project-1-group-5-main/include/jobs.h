#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

#define MAX_JOBS 10
#define MAX_PIPE_PIDS 3

typedef struct {
    int   active;
    int   job_no;                 /* monotonically increasing, never reused */
    int   npids;                  /* 1..3 */
    pid_t pids[MAX_PIPE_PIDS];    /* track all children in the job */
    char *cmdline;                /* as typed (without trailing &) */
} Job;

void jobs_init(void);
int  jobs_has_capacity(void);
int  jobs_add(int npids, const pid_t *pids, const char *cmdline); /* prints start line, returns job# or -1 */
void jobs_poll(void);   /* reap finished with WNOHANG; prints “[job] + done …” */
void jobs_wait_all(void); /* block until all active jobs finish (for exit builtin later) */

/* (Optional for Part 9) list active jobs */
void jobs_list(void);

#endif
