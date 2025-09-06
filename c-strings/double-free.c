#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	char *hello = malloc(strlen("Hello World!\n") + 1);
	// strcpy(hello, "Hello World!\n");
	strncpy(hello, "Hello World!\n", strlen("Hello World!\n") + 1);
	printf("%s", hello);
	free(hello);
	// nothing to free at address, so this will cause a double free error.
	free(hello);
	return 0;
}
