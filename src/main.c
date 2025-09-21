#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"
#include "parse.h"
#include "builtins.h"
#include "exec.h"
#include "jobs.h"

static void print_prompt(void){
    char cwd[512]; getcwd(cwd, sizeof(cwd));
    const char *user = getenv("USER"); if(!user) user="user";
    char host[128] = "host"; gethostname(host, sizeof(host)-1);
    printf("%s@%s:%s> ", user, host, cwd); fflush(stdout);
}

int main(void){
    jobs_init();
    char line[MAX_CMDLINE];
    while (1){
        jobs_mark_done_poll_and_print();
        print_prompt();
        if (!fgets(line, sizeof(line), stdin)) break;
        if (line[0]=='\n') continue;
        CommandPlan plan = {0};
        strncpy(plan.rawline, line, sizeof(plan.rawline)-1);
        if (parse_line(line, &plan) != 0) { fprintf(stderr,"parse error\n"); continue; }
        if (!maybe_run_builtin(&plan)) {
            if (execute_plan(&plan) != 0) fprintf(stderr,"exec error\n");
            else history_add(plan.rawline);
        }
    }
    return 0;
}
