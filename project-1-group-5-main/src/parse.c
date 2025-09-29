#include "parse.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static char *xstrdup_local(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char *p = (char *)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n + 1);
    return p;
}

/* Remove at most ONE '< file' and ONE '> file' from tokens (wherever they appear).
 * Store heap-duplicated paths in *in_path / *out_path (caller frees).
 * Returns 0 on success; -1 on syntax error (missing filename, duplicates).
 */
int extract_redirections(tokenlist *toks, char **in_path, char **out_path) {
    if (!toks) return 0;
    if (in_path)  *in_path  = NULL;
    if (out_path) *out_path = NULL;

    size_t i = 0;
    while (i < toks->size) {
        char *tok = toks->items[i];
        if (tok && (strcmp(tok, "<") == 0 || strcmp(tok, ">") == 0)) {
            int is_in = (tok[0] == '<');
            if (i + 1 >= toks->size) return -1;           /* missing filename */
            char *fname = toks->items[i + 1];
            if (!fname || strcmp(fname, "|") == 0 || strcmp(fname, "<") == 0 || strcmp(fname, ">") == 0)
                return -1;                                 /* invalid filename token */

            if (is_in) {
                if (in_path && *in_path) return -1;        /* duplicate '<' */
                if (in_path) {
                    *in_path = xstrdup_local(fname);
                    if (!*in_path) return -1;
                }
            } else {
                if (out_path && *out_path) return -1;      /* duplicate '>' */
                if (out_path) {
                    *out_path = xstrdup_local(fname);
                    if (!*out_path) return -1;
                }
            }

            /* remove the pair tok,fname at indexes i,i+1 */
            free(toks->items[i]);      /* "<" or ">" */
            free(toks->items[i + 1]);  /* filename */
            for (size_t j = i + 2; j < toks->size; j++) {
                toks->items[j - 2] = toks->items[j];
            }
            toks->size -= 2;
            toks->items[toks->size] = NULL;
            continue;  /* keep i at same index to examine the shifted token */
        }
        i++;
    }
    return 0;
}



/* Remove a trailing '&' from the *last* token.
 * Accepts either a standalone "&" token *or* an ampersand stuck to the end
 * of the last token (e.g., "10000&"). Returns 1 if background requested. */
int extract_background(tokenlist *toks) {
    if (!toks || toks->size == 0) return 0;

    size_t i = toks->size - 1;
    char *last = toks->items[i];
    if (!last) return 0;

    /* Case 1: last token is exactly "&" -> drop that token */
    if (strcmp(last, "&") == 0) {
        free(toks->items[i]);
        toks->items[i] = NULL;
        toks->size -= 1;
        return 1;
    }

    /* Case 2: ampersand stuck at end, e.g., "10000&" or "sleep&" */
    size_t len = strlen(last);
    if (len > 0 && last[len - 1] == '&') {
        /* Trim the trailing '&' in-place */
        last[len - 1] = '\0';
        /* If the whole token was just "&" (len==1), Case 1 already handled */
        return 1;
    }

    return 0;
}



/* remove token at index idx, shift left, keep NULL-termination */
static void remove_token(tokenlist *t, size_t idx) {
    if (!t || idx >= t->size) return;
    free(t->items[idx]);
    for (size_t i = idx; i + 1 < t->size; i++) {
        t->items[i] = t->items[i + 1];
    }
    t->size -= 1;
    t->items[t->size] = NULL;
}