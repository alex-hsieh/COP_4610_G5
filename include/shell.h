#pragma once
#include <stdbool.h>
#define MAX_ARGS 128
#define MAX_CMDS 3
#define MAX_CMDLINE 256

typedef struct {
    char *in_file;
    char *out_file;
    bool  background;
    char *argv[MAX_ARGS]; // NULL-terminated
} SimpleCmd;

typedef struct {
    int   ncmds;                 // 1..3
    SimpleCmd cmds[MAX_CMDS];
    char  rawline[MAX_CMDLINE];
} CommandPlan;
