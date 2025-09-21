#pragma once
#include "shell.h"
int maybe_run_builtin(CommandPlan *plan); // 1 ran builtin, 0 not a builtin
void history_add(const char *line);
