#define _POSIX_C_SOURCE 200809L
#include "exec.h"
#include "path.h"

#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

/* ---------- helpers ---------- */

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
    /* Create or truncate; permissions -rw------- (0600) */
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (fd < 0) return -1;
    (void)fchmod(fd, 0600); /* ensure exact perms in case umask masked */
    return fd;
}

/* ---------- foreground exec with optional redirections ---------- */

int run_external_with_redirs(tokenlist *toks, const char *in_path, const char *out_path) {
    if (!toks || toks->size == 0 || !toks->items || !toks->items[0]) return 0;

    char full[PATH_MAX];
    if (resolve_command_path(toks->items[0], full) != 0) {
        fprintf(stderr, "command not found: %s\n", toks->items[0]);
        return 127;
    }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return -1; }

    if (pid == 0) {
        /* child: setup redirections */
        if (in_path) {
            int infd = open_input_fd(in_path);
            if (infd < 0) { perror("input redirection"); _exit(1); }
            if (dup2(infd, STDIN_FILENO) < 0) { perror("dup2 stdin"); _exit(1); }
            close(infd);
        }
        if (out_path) {
            int outfd = open_output_fd(out_path);
            if (outfd < 0) { perror("output redirection"); _exit(1); }
            if (dup2(outfd, STDOUT_FILENO) < 0) { perror("dup2 stdout"); _exit(1); }
            close(outfd);
        }

        execv(full, toks->items);
        perror("execv");
        _exit(127);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) { perror("waitpid"); return -1; }
    return (WIFEXITED(status) ? WEXITSTATUS(status) : -1);
}

int run_external(tokenlist *toks) {
    return run_external_with_redirs(toks, NULL, NULL);
}

/* ---------- background spawn with optional redirections ---------- */

int spawn_external_with_redirs_bg(tokenlist *toks, const char *in_path, const char *out_path, pid_t *child_pid) {
    if (!toks || toks->size == 0 || !toks->items || !toks->items[0]) return -1;

    char full[PATH_MAX];
    if (resolve_command_path(toks->items[0], full) != 0) {
        fprintf(stderr, "command not found: %s\n", toks->items[0]);
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return -1; }

    if (pid == 0) {
        int infd = -1, outfd = -1;

        if (in_path) {
            infd = open_input_fd(in_path);
            if (infd < 0) { perror("input redirection"); _exit(1); }
            if (dup2(infd, STDIN_FILENO) < 0) { perror("dup2 stdin"); _exit(1); }
            close(infd);
        }
        if (out_path) {
            outfd = open_output_fd(out_path);
            if (outfd < 0) { perror("output redirection"); _exit(1); }
            if (dup2(outfd, STDOUT_FILENO) < 0) { perror("dup2 stdout"); _exit(1); }
            close(outfd);
        }

        execv(full, toks->items);
        perror("execv");
        _exit(127);
    }

    if (child_pid) *child_pid = pid;
    return 0;
}

