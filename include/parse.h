#pragma once
#include "shell.h"
int parse_line(const char *line, CommandPlan *out); // 0 ok, nonzero error
