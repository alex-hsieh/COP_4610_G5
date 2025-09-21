#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include "exec.h"
#include "path.h"

int execute_plan(CommandPlan *p){
    char **argv = p->cmds[0].argv;
    char *prog = argv[0];
    char *full = prog;
    pid_t pid = fork();
    if (pid == 0){
        execv(full, argv);
        perror("execv");
        _exit(127);
    }
    int st; waitpid(pid, &st, 0);
    return 0;
}
