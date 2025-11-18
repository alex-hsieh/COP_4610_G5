#ifndef EXPAND_H
#define EXPAND_H

#include "lexer.h"  // for tokenlist


/* Replace tokens like "$USER" -> "actual_value" ("" if unset). */
void expand_tilde_tokens(tokenlist *tokens);
void expand_env_tokens(tokenlist *tokens);



#endif
