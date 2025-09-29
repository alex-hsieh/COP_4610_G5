#ifndef HISTORY_H
#define HISTORY_H

void history_init(void);
void history_record(const char *cmdline);  // record any non-empty executed cmd
void history_print_on_exit(void);          // prints last 3 (or last 1, or none)

#endif
