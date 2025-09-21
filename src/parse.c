#include <string.h>
#include "parse.h"
int parse_line(const char *line, CommandPlan *out){
    (void)line;
    memset(out, 0, sizeof(*out));

    out->ncmds = 1;
    out->cmds[0].argv[0] = "echo";
    out->cmds[0].argv[1] = "stub";
    out->cmds[0].argv[2] = NULL;
    return 0;
}
