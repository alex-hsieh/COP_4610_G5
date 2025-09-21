#pragma once
void jobs_init(void);
void jobs_mark_done_poll_and_print(void);
void jobs_print_active(void);
int  jobs_add(int pgid, const char *cmdline);
