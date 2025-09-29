#define _POSIX_C_SOURCE 200809L
#include "jobs.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static Job table[MAX_JOBS];
static int initialized = 0;
static int next_job_no = 1;

void jobs_init(void) {
    if (initialized) return;
    memset(table, 0, sizeof(table));
    initialized = 1;
}

static int all_reaped(const Job *j) {
    for (int i = 0; i < j->npids; i++) if (j->pids[i] != -1) return 0;
    return 1;
}

int jobs_has_capacity(void) {
    for (int i = 0; i < MAX_JOBS; i++) if (!table[i].active) return 1;
    return 0;
}

int jobs_add(int npids, const pid_t *pids, const char *cmdline) {
    if (!initialized) jobs_init();
    int slot = -1;
    for (int i = 0; i < MAX_JOBS; i++) if (!table[i].active) { slot = i; break; }
    if (slot < 0) return -1;

    Job *j = &table[slot];
    j->active = 1;
    j->job_no = next_job_no++;
    j->npids  = npids;
    for (int i = 0; i < npids; i++) j->pids[i] = pids[i];
    j->cmdline = cmdline ? strdup(cmdline) : strdup("");

    /* print start line with LAST stage pid */
    pid_t last = j->pids[npids - 1];
    printf("[%d] %d\n", j->job_no, (int)last);
    fflush(stdout);
    return j->job_no;
}

void jobs_poll(void) {
    if (!initialized) return;

    /* First: check each tracked PID non-blocking */
    for (int i = 0; i < MAX_JOBS; i++) {
        Job *j = &table[i];
        if (!j->active) continue;

        for (int k = 0; k < j->npids; k++) {
            pid_t p = j->pids[k];
            if (p == -1) continue;

            int status = 0;
            pid_t r = waitpid(p, &status, WNOHANG);
            if (r == p) {
                j->pids[k] = -1;  /* reaped */
            } else if (r < 0 && errno == ECHILD) {
                j->pids[k] = -1;  /* already reaped elsewhere */
            }
        }

        if (all_reaped(j)) {
            /* completion line */
            printf("[%d] + done %s\n", j->job_no, j->cmdline ? j->cmdline : "");
            fflush(stdout);
            free(j->cmdline);
            memset(j, 0, sizeof(*j));
        }
    }

    /* Then: drain any other finished children (e.g., non-last pipeline stages not tracked in a job) */
    int status = 0;
    while (1) {
        pid_t r = waitpid(-1, &status, WNOHANG);
        if (r <= 0) break;
        /* deliberately ignore; they belong to currently-running foreground tasks or already-closed bg stages */
    }
}

void jobs_wait_all(void) {
    if (!initialized) return;
    for (int i = 0; i < MAX_JOBS; i++) {
        Job *j = &table[i];
        if (!j->active) continue;

        /* block until all pids finish */
        for (int k = 0; k < j->npids; k++) {
            if (j->pids[k] != -1) {
                (void)waitpid(j->pids[k], NULL, 0);
                j->pids[k] = -1;
            }
        }

        /* print the same completion line we print in jobs_poll() */
        printf("[%d] + done %s\n", j->job_no, j->cmdline ? j->cmdline : "");
        fflush(stdout);

        if (j->cmdline) free(j->cmdline);
        memset(j, 0, sizeof(*j));
    }
}


void jobs_list(void) {
    if (!initialized) return;
    int any = 0;
    for (int i = 0; i < MAX_JOBS; i++) {
        Job *j = &table[i];
        if (!j->active) continue;
        any = 1;
        printf("[%d]+ %d %s\n", j->job_no, (int)j->pids[j->npids - 1], j->cmdline ? j->cmdline : "");
    }
    if (!any) puts("no active background processes");
    fflush(stdout);
}
