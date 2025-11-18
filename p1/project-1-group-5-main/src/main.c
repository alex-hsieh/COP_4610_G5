// src/main.c
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "shell.h"
#include "lexer.h"
#include "expand.h"
#include "parse.h"
#include "exec.h"
#include "pipeline.h"
#include "jobs.h"
#include "history.h"
#include "builtins.h"
#include "path.h"

static char *trim_trailing_ampersand(const char *line) {
    if (!line) return NULL;
    size_t n = strlen(line);
    while (n > 0 && isspace((unsigned char)line[n - 1])) n--;
    if (n > 0 && line[n - 1] == '&') { n--; while (n > 0 && isspace((unsigned char)line[n - 1])) n--; }
    char *out = (char *)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, line, n); out[n] = '\0'; return out;
}

static int strip_background_from_tokens(tokenlist *toks) {
    if (!toks || toks->size == 0) return 0;
    size_t i = toks->size - 1; char *last = toks->items[i];
    if (!last) return 0;
    if (strcmp(last, "&") == 0) { free(toks->items[i]); toks->items[i]=NULL; toks->size--; return 1; }
    size_t len = strlen(last);
    if (len > 0 && last[len - 1] == '&') { last[len - 1] = '\0'; return 1; }
    return 0;
}

int main(void) {
    jobs_init();
    history_init();

    for (;;) {
        jobs_poll();
        update_pwd_env();
        print_prompt();

        char *line = get_input();
        if (!line) { puts(""); break; }
        if (line[0] == '\0') { free(line); continue; }

        tokenlist *toks = get_tokens(line);
        expand_tilde_tokens(toks);
        expand_env_tokens(toks);

        int background = strip_background_from_tokens(toks);
        char *cmdline_for_job = background ? trim_trailing_ampersand(line) : NULL;

        int should_quit = 0;

        if (toks->size > 0 && is_builtin(toks)) {
            const char *cmd = toks->items[0];
            if (strcmp(cmd, "exit") != 0) history_record(cmdline_for_job ? cmdline_for_job : line);

            if (strcmp(cmd, "cd") == 0) {
                (void)builtin_cd(toks);
            } else if (strcmp(cmd, "jobs") == 0) {
                (void)builtin_jobs();
            } else if (strcmp(cmd, "exit") == 0) {
                should_quit = builtin_exit_and_should_quit();
            }

        } else if (toks->size > 0 && toks->items[0]) {
            /* NEW: always extract redirections, even if a pipeline exists. */
            char *in_path = NULL, *out_path = NULL;
            if (extract_redirections(toks, &in_path, &out_path) != 0) {
                fprintf(stderr, "redirection syntax error\n");
                free(in_path); free(out_path);
            } else {
                if (contains_pipe(toks)) {
                    /* Pipeline: record entire command in history */
                    history_record(cmdline_for_job ? cmdline_for_job : line);

                    if (background) {
                        if (!jobs_has_capacity()) {
                            fprintf(stderr, "too many background processes\n");
                            (void)run_pipeline(toks, in_path, out_path);
                        } else {
                            pid_t lastpid[3]; int np=0;
                            if (run_pipeline_bg(toks, in_path, out_path, lastpid, &np) == 0) {
                                jobs_add(1, lastpid, cmdline_for_job ? cmdline_for_job : "");
                            }
                        }
                    } else {
                        (void)run_pipeline(toks, in_path, out_path);
                    }

                } else {
                    /* Single external: record only if it resolves */
                    char full[PATH_MAX];
                    int resolves = (resolve_command_path(toks->items[0], full) == 0);
                    if (background) {
                        if (!jobs_has_capacity()) {
                            fprintf(stderr, "too many background processes\n");
                            if (resolves) history_record(cmdline_for_job ? cmdline_for_job : line);
                            (void)run_external_with_redirs(toks, in_path, out_path);
                        } else {
                            if (resolves) history_record(cmdline_for_job ? cmdline_for_job : line);
                            pid_t cpid=-1;
                            if (spawn_external_with_redirs_bg(toks, in_path, out_path, &cpid) == 0 && resolves) {
                                pid_t one[1]={cpid};
                                jobs_add(1, one, cmdline_for_job ? cmdline_for_job : "");
                            }
                        }
                    } else {
                        if (resolves) history_record(cmdline_for_job ? cmdline_for_job : line);
                        (void)run_external_with_redirs(toks, in_path, out_path);
                    }
                }
                free(in_path); free(out_path);
            }
        }

        free(cmdline_for_job);
        free_tokens(toks);
        free(line);

        if (should_quit) break;
    }

    jobs_wait_all();
    return 0;
}
