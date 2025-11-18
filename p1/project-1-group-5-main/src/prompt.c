#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "shell.h"

static const char *fallback(const char *v, const char *fb) {
    return (v && *v) ? v : fb;
}

void update_pwd_env(void) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        /* keep $PWD accurate for the prompt */
        setenv("PWD", cwd, 1);
    }
}

void print_prompt(void) {
    /* USER from $USER, fall back to getlogin() */
    const char *user = getenv("USER");
    if (!user || !*user) user = getlogin();

    /* MACHINE: prefer $MACHINE; else use hostname */
    const char *machine = getenv("MACHINE");
    char hostbuf[256] = "unknown";
    if (!machine || !*machine) {
        if (gethostname(hostbuf, sizeof(hostbuf) - 1) == 0) {
            hostbuf[sizeof(hostbuf) - 1] = '\0';
            machine = hostbuf;
        } else {
            machine = "unknown";
        }
    }

    /* PWD maintained by update_pwd_env() */
    const char *pwd = getenv("PWD");

    printf("%s@%s:%s> ", 
           fallback(user, "unknown"),
           fallback(machine, "unknown"),
           fallback(pwd, "?"));
    fflush(stdout);
}
