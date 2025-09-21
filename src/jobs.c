#include <stdio.h>
#include "jobs.h"
void jobs_init(void){}
void jobs_mark_done_poll_and_print(void){}
void jobs_print_active(void){ printf("No active background processes.\n"); }
int  jobs_add(int pgid, const char *cmdline){ (void)pgid; (void)cmdline; return 1; }
