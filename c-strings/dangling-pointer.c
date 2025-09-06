#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	char *hello = malloc(strlen("Hello World!\n") + 1);
	strncpy(hello, "Hello World!\n", strlen("Hello World!\n") + 1);
	free(hello);
	// hello is now a dangling pointer, it points to memory that we don't have access to.
	printf("%s", hello);
	return 0;
}
