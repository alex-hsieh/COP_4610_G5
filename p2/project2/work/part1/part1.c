#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main() {
    char buffer[1024];
    getpid();
    getppid();
    getuid();
    getgid();
    getcwd(buffer, sizeof(buffer));
    return 0;
}
