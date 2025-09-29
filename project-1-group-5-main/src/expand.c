#define _POSIX_C_SOURCE 200809L
#include "expand.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

/* Expand only "~" and "~/..." to $HOME */
void expand_tilde_tokens(tokenlist *tokens) {
    if (!tokens || !tokens->items) return;

    /* get HOME, with passwd fallback */
    const char *home = getenv("HOME");
    if (!home || !*home) {
        struct passwd *pw = getpwuid(getuid());
        if (pw && pw->pw_dir) home = pw->pw_dir;
    }
    if (!home) home = "";

    for (size_t i = 0; i < tokens->size; i++) {
        char *tok = tokens->items[i];
        if (!tok) continue;

        /* Only if token is "~" or starts with "~/" */
        if (tok[0] == '~' && (tok[1] == '\0' || tok[1] == '/')) {
            const char *suffix = (tok[1] == '/') ? (tok + 1) : "";  // keep the "/" if present
            size_t need = strlen(home) + strlen(suffix) + 1;

            char *rep = (char *)malloc(need);
            if (!rep) continue;  // OOM: leave token unchanged

            strcpy(rep, home);
            strcat(rep, suffix);

            free(tokens->items[i]);
            tokens->items[i] = rep;
        }
    }
}

void expand_env_tokens(tokenlist *tokens) {
    if (!tokens || !tokens->items) return;

    for (size_t i = 0; i < tokens->size; i++) {
        char *tok = tokens->items[i];
        if (!tok) continue;

        /* Only expand if the whole token starts with '$' and has a name after it */
        if (tok[0] == '$' && tok[1] != '\0') {
            const char *name = tok + 1;                /* everything after '$' */
            const char *val  = getenv(name);           /* NULL if not set */
            if (!val) val = "";                        /* bash-like: empty if unset */

            /* Replace token contents with the expanded value */
            char *rep = (char *)malloc(strlen(val) + 1);
            if (!rep) continue;                        /* out of mem? leave token as-is */
            strcpy(rep, val);
            free(tokens->items[i]);
            tokens->items[i] = rep;
        }
        /* else: leave token as-is, e.g., "$" alone or non-$ tokens */
    }
}
