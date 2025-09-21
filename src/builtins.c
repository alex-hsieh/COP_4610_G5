#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "builtins.h"
#include "jobs.h"

static char* last[3] = {0};

void history_add(const char *line){
    if (last[2]) free(last[2]);
    last[2]=last[1]; last[1]=last[0]; last[0]=strdup(line);
}

int maybe_run_builtin(CommandPlan *p){
    if (p->ncmds!=1 || !p->cmds[0].argv[0]) return 0;
    if (strcmp(p->cmds[0].argv[0],"jobs")==0){ jobs_print_active(); return 1; }
    if (strcmp(p->cmds[0].argv[0],"exit")==0){

        if (last[0]) printf("%s", last[0]); else printf("No valid commands.\n");
        exit(0);
    }
    return 0;
}
