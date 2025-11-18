#ifndef EXEC_H
#define EXEC_H

#include <sys/types.h>
#include "lexer.h"

/* Execute a non-builtin command (no redirections). */
int run_external(tokenlist *toks);

/* Execute with optional input/output redirections (foreground). */
int run_external_with_redirs(tokenlist *toks, const char *in_path, const char *out_path);

/* Spawn in background with optional redirections; returns child's pid via child_pid. */
int spawn_external_with_redirs_bg(tokenlist *toks, const char *in_path, const char *out_path, pid_t *child_pid);

#endif

