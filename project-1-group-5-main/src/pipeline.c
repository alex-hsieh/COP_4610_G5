#define _POSIX_C_SOURCE 200809L
#include "pipeline.h"
#include "path.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>

/* ---------- utils ---------- */

int contains_pipe(const tokenlist *t) {
    if (!t) return 0;
    for (size_t i = 0; i < t->size; i++)
        if (t->items[i] && strcmp(t->items[i], "|") == 0) return 1;
    return 0;
}

static int open_input_fd(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    struct stat st;
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {
        close(fd);
        errno = EINVAL;
        return -1;
    }
    return fd;
}

static int open_output_fd(const char *path) {
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (fd < 0) return -1;
    (void)fchmod(fd, 0600);
    return fd;
}

/* Build argv arrays for N stages where N = number of '|' + 1
 * Returns 0 on success; -1 on syntax error or OOM.
 * On success: *argvs is array of N argv vectors (each NULL-terminated).
 */
static int split_into_stages_any(tokenlist *t, char ****out_argvs, int *out_n) {
    if (!t || t->size == 0) return -1;

    int n = 1;
    for (size_t i = 0; i < t->size; i++)
        if (t->items[i] && strcmp(t->items[i], "|") == 0) n++;

    if (n <= 1) return -1;

    int *argc = (int *)calloc(n, sizeof(int));
    if (!argc) return -1;

    int s = 0;
    for (size_t i = 0; i < t->size; i++) {
        char *tok = t->items[i];
        if (tok && strcmp(tok, "|") == 0) { s++; continue; }
        argc[s]++;
    }
    for (int i = 0; i < n; i++) {
        if (argc[i] == 0) { free(argc); return -1; }
    }

    char ***argvs = (char ***)calloc(n, sizeof(char **));
    if (!argvs) { free(argc); return -1; }
    for (int i = 0; i < n; i++) {
        argvs[i] = (char **)calloc(argc[i] + 1, sizeof(char *));
        if (!argvs[i]) {
            for (int k = 0; k < i; k++) free(argvs[k]);
            free(argvs); free(argc); return -1;
        }
    }

    s = 0; int idx = 0;
    for (size_t i = 0; i < t->size; i++) {
        char *tok = t->items[i];
        if (tok && strcmp(tok, "|") == 0) {
            argvs[s][idx] = NULL; s++; idx = 0; continue;
        }
        argvs[s][idx++] = t->items[i];
    }
    argvs[s][idx] = NULL;

    free(argc);
    *out_argvs = argvs;
    *out_n = n;
    return 0;
}

static void free_stage_argvs(char ***argvs, int n) {
    if (!argvs) return;
    for (int i = 0; i < n; i++) free(argvs[i]);
    free(argvs);
}

static void close_all_pipes(int (*pipes)[2], int nminus1) {
    if (!pipes) return;
    for (int i = 0; i < nminus1; i++) {
        if (pipes[i][0] != -1) close(pipes[i][0]);
        if (pipes[i][1] != -1) close(pipes[i][1]);
    }
}

/* ---------- foreground pipeline (supports < on stage 0, > on last stage) ---------- */

int run_pipeline(tokenlist *t, const char *in_path, const char *out_path) {
    char ***argvs = NULL; int n = 0;
    if (split_into_stages_any(t, &argvs, &n) != 0) {
        fprintf(stderr, "pipeline syntax error\n");
        return -1;
    }

    int (*pipes)[2] = NULL;
    if (n > 1) {
        pipes = (int (*)[2])calloc((size_t)(n - 1), sizeof(int[2]));
        if (!pipes) { free_stage_argvs(argvs, n); return -1; }
        for (int i = 0; i < n - 1; i++) {
            if (pipe(pipes[i]) < 0) {
                perror("pipe");
                for (int j = 0; j < i; j++) { close(pipes[j][0]); close(pipes[j][1]); }
                free(pipes); free_stage_argvs(argvs, n); return -1;
            }
        }
    }

    pid_t *pids = (pid_t *)calloc(n, sizeof(pid_t));
    if (!pids) { close_all_pipes(pipes, n - 1); free(pipes); free_stage_argvs(argvs, n); return -1; }

    for (int s = 0; s < n; s++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close_all_pipes(pipes, n - 1); free(pipes); free_stage_argvs(argvs, n); free(pids);
            return -1;
        }
        if (pid == 0) {
            /* child wiring: pipes */
            if (n > 1) {
                if (s > 0) {
                    if (dup2(pipes[s - 1][0], STDIN_FILENO) < 0) { perror("dup2"); _exit(1); }
                }
                if (s < n - 1) {
                    if (dup2(pipes[s][1], STDOUT_FILENO) < 0) { perror("dup2"); _exit(1); }
                }
            }
            /* redirections on first/last stage */
            if (s == 0 && in_path) {
                int infd = open_input_fd(in_path);
                if (infd < 0) { perror("input redirection"); _exit(1); }
                if (dup2(infd, STDIN_FILENO) < 0) { perror("dup2 stdin"); _exit(1); }
                close(infd);
            }
            if (s == n - 1 && out_path) {
                int outfd = open_output_fd(out_path);
                if (outfd < 0) { perror("output redirection"); _exit(1); }
                if (dup2(outfd, STDOUT_FILENO) < 0) { perror("dup2 stdout"); _exit(1); }
                close(outfd);
            }
            /* close all pipes in child */
            close_all_pipes(pipes, n - 1);

            char full[PATH_MAX];
            if (resolve_command_path(argvs[s][0], full) != 0) {
                fprintf(stderr, "command not found: %s\n", argvs[s][0]);
                _exit(127);
            }
            execv(full, argvs[s]);
            perror("execv"); _exit(127);
        }
        pids[s] = pid;
    }

    close_all_pipes(pipes, n - 1); free(pipes);

    int status = 0, last = -1;
    for (int s = 0; s < n; s++) {
        if (waitpid(pids[s], &status, 0) >= 0)
            if (s == n - 1 && WIFEXITED(status)) last = WEXITSTATUS(status);
    }
    free(pids); free_stage_argvs(argvs, n);
    return last;
}

/* ---------- background pipeline (returns last pid in out_pids[0]) ---------- */

int run_pipeline_bg(tokenlist *t, const char *in_path, const char *out_path, pid_t out_pids[3], int *out_npids) {
    char ***argvs = NULL; int n = 0;
    if (split_into_stages_any(t, &argvs, &n) != 0) {
        fprintf(stderr, "pipeline syntax error\n"); return -1;
    }

    int (*pipes)[2] = NULL;
    if (n > 1) {
        pipes = (int (*)[2])calloc((size_t)(n - 1), sizeof(int[2]));
        if (!pipes) { free_stage_argvs(argvs, n); return -1; }
        for (int i = 0; i < n - 1; i++) {
            if (pipe(pipes[i]) < 0) {
                perror("pipe");
                for (int j = 0; j < i; j++) { close(pipes[j][0]); close(pipes[j][1]); }
                free(pipes); free_stage_argvs(argvs, n); return -1;
            }
        }
    }

    pid_t *pids = (pid_t *)calloc(n, sizeof(pid_t));
    if (!pids) { close_all_pipes(pipes, n - 1); free(pipes); free_stage_argvs(argvs, n); return -1; }

    for (int s = 0; s < n; s++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close_all_pipes(pipes, n - 1); free(pipes); free_stage_argvs(argvs, n); free(pids);
            return -1;
        }
        if (pid == 0) {
            if (n > 1) {
                if (s > 0) {
                    if (dup2(pipes[s - 1][0], STDIN_FILENO) < 0) { perror("dup2"); _exit(1); }
                }
                if (s < n - 1) {
                    if (dup2(pipes[s][1], STDOUT_FILENO) < 0) { perror("dup2"); _exit(1); }
                }
            }
            if (s == 0 && in_path) {
                int infd = open_input_fd(in_path);
                if (infd < 0) { perror("input redirection"); _exit(1); }
                if (dup2(infd, STDIN_FILENO) < 0) { perror("dup2 stdin"); _exit(1); }
                close(infd);
            }
            if (s == n - 1 && out_path) {
                int outfd = open_output_fd(out_path);
                if (outfd < 0) { perror("output redirection"); _exit(1); }
                if (dup2(outfd, STDOUT_FILENO) < 0) { perror("dup2 stdout"); _exit(1); }
                close(outfd);
            }
            close_all_pipes(pipes, n - 1);

            char full[PATH_MAX];
            if (resolve_command_path(argvs[s][0], full) != 0) {
                fprintf(stderr, "command not found: %s\n", argvs[s][0]);
                _exit(127);
            }
            execv(full, argvs[s]);
            perror("execv"); _exit(127);
        }
        pids[s] = pid;
    }

    close_all_pipes(pipes, n - 1); free(pipes);

    if (out_pids) out_pids[0] = pids[n - 1];
    if (out_npids) *out_npids = 1;

    free(pids); free_stage_argvs(argvs, n);
    return 0;
}
