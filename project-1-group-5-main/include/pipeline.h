#ifndef PIPELINE_H
#define PIPELINE_H

#include <sys/types.h>
#include "lexer.h"

/* Return 1 if any '|' token present, else 0 */
int contains_pipe(const tokenlist *t);

/* Execute a pipeline with ANY number of stages; supports optional in/out files on first/last stage. */
int run_pipeline(tokenlist *t, const char *in_path, const char *out_path);

/* Background pipeline:
 *  - launches without waiting
 *  - returns last stage's PID in out_pids[0] and sets *out_npids = 1 (so jobs code stays unchanged)
 */
int run_pipeline_bg(tokenlist *t, const char *in_path, const char *out_path, pid_t out_pids[3], int *out_npids);

#endif
