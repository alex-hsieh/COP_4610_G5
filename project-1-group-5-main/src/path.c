#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>      // <-- add this for snprintf
#include <limits.h>

#include "path.h"
#include "lexer.h"

/* Internal helper: candidate must exist, be executable, and be a regular file. */
static int is_exec_regular(const char *p) {
    struct stat st;
    if (access(p, X_OK) != 0) return -1;
    if (stat(p, &st) != 0) return -1;
    return S_ISREG(st.st_mode) ? 0 : -1;
}

int resolve_command_path(const char *cmd, char out[PATH_MAX]) {
    if (!cmd || !*cmd) return -1;

    /* If command has a slash, treat as explicit path */
    if (strchr(cmd, '/')) {
        /* No normalization; just check it's executable and regular */
        if (is_exec_regular(cmd) == 0) {
            strncpy(out, cmd, PATH_MAX - 1);
            out[PATH_MAX - 1] = '\0';
            return 0;
        }
        return -1;
    }

    const char *path = getenv("PATH");
    if (!path) path = "";

    /* We **don’t** use strtok so we correctly handle empty segments (meaning ".") */
    const char *seg = path;
    while (1) {
        const char *colon = strchr(seg, ':');
        size_t len = colon ? (size_t)(colon - seg) : strlen(seg);

        /* Empty segment → current directory "." */
        char dir[PATH_MAX];
        if (len == 0) {
            strcpy(dir, ".");  /* safe */
        } else {
            if (len >= sizeof(dir)) len = sizeof(dir) - 1;
            memcpy(dir, seg, len);
            dir[len] = '\0';
        }

        char candidate[PATH_MAX];
        int nw = snprintf(candidate, sizeof(candidate), "%s/%s", dir, cmd);
        if (nw > 0 && (size_t)nw < sizeof(candidate)) {
            if (is_exec_regular(candidate) == 0) {
                strncpy(out, candidate, PATH_MAX - 1);
                out[PATH_MAX - 1] = '\0';
                return 0;
            }
        }

        if (!colon) break;
        seg = colon + 1;
    }

    return -1; /* not found */
}
