#define _POSIX_C_SOURCE 200809L
#include "history.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_KEEP 32
#define MAX_CMD  200

static char *ring[MAX_KEEP];
static int count = 0;

void history_init(void) {
    for (int i = 0; i < MAX_KEEP; i++) ring[i] = NULL;
    count = 0;
}

void history_record(const char *cmdline) {
    if (!cmdline || !*cmdline) return;
    size_t n = strnlen(cmdline, MAX_CMD);
    char *cpy = (char *)malloc(n + 1);
    if (!cpy) return;
    memcpy(cpy, cmdline, n);
    cpy[n] = '\0';

    int idx = count % MAX_KEEP;
    if (ring[idx]) free(ring[idx]);
    ring[idx] = cpy;
    count++;
}

/* Spec text: if >=3, print last three; if <3 but >=1, print the last one; if none, say so. */
void history_print_on_exit(void) {
    if (count == 0) {
        puts("no valid commands");
        return;
    }
    if (count >= 3) {
        for (int i = 3; i >= 1; i--) {
            int idx = (count - i) % MAX_KEEP;
            printf("%s\n", ring[idx]);
        }
    } else {
        int idx = (count - 1) % MAX_KEEP;
        printf("%s\n", ring[idx]);
    }
    /* optional: free memory */
    for (int i = 0; i < MAX_KEEP; i++) { free(ring[i]); ring[i] = NULL; }
}
