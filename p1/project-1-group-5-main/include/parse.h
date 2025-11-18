#ifndef PARSE_H
#define PARSE_H

#include "lexer.h"

/* Scan tokens for '<' and '>' and extract filenames.
 * Removes the redirection tokens from the tokenlist in-place.
 * On success, *in_path / *out_path are malloc'ed (caller frees) or NULL.
 * Returns 0 on success, -1 on syntax error (e.g., missing filename).
 */
int extract_background(tokenlist *toks);
int extract_redirections(tokenlist *toks, char **in_path, char **out_path);

#endif
