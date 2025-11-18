#ifndef BUILTINS_H
#define BUILTINS_H

#include "lexer.h"

int is_builtin(const tokenlist *t);
int builtin_cd(const tokenlist *t);
int builtin_jobs(void);
int builtin_exit_and_should_quit(void);  // waits for bg, prints history, returns 1 to tell main to exit

#endif
