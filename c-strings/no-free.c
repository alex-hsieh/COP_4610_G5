#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// run valgrind!
int main() {
	char *hello = malloc(strlen("Hello World!\n") + 1);
	//snprintf(hello, strlen("Hello World!\n") + 1, "Hello World!\n");
	strncpy(hello, "Hello World!\n", strlen("Hello World!\n") + 1);
	//strcpy(hello, "Hello World!\n");
	printf("%s", hello);
	// free(hello);
	return 0;
}
