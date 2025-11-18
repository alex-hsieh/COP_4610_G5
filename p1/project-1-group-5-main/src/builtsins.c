#define _POSIX_C_SOURCE 200809L
#include "builtins.h"
#include "shell.h"
#include "jobs.h"
#include "history.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

static int is_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
}

int is_builtin(const tokenlist *t) {
    if (!t || t->size == 0 || !t->items[0]) return 0;
    const char *cmd = t->items[0];
    return (strcmp(cmd, "cd") == 0) || (strcmp(cmd, "exit") == 0) || (strcmp(cmd, "jobs") == 0);
}

int builtin_cd(const tokenlist *t) {
    if (!t || t->size == 0) return -1;

    if (t->size > 2) { fprintf(stderr, "cd: too many arguments\n"); return -1; }

    const char *target = NULL;
    if (t->size == 1) {
        target = getenv("HOME");
        if (!target || !*target) { fprintf(stderr, "cd: HOME not set\n"); return -1; }
    } else {
        target = t->items[1];
    }

    if (!is_directory(target)) {
        fprintf(stderr, "cd: not a directory or does not exist: %s\n", target);
        return -1;
    }
    if (chdir(target) != 0) {
        perror("cd");
        return -1;
    }
    update_pwd_env();  // keep $PWD accurate for the prompt
    return 0;
}

int builtin_jobs(void) {
    jobs_list();
    return 0;
}

int builtin_exit_and_should_quit(void) {
    /* wait for all bg jobs, then print history lines per spec */
    jobs_wait_all();
    history_print_on_exit();
    return 1;  // tell main to terminate
}
