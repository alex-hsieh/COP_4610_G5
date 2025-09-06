// give an example of memory reallocation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	// give example of memory rellocartion
	char *hello = malloc(strlen("Hello") + 1);
	strncpy(hello, "Hello", strlen("Hello") + 1);
	printf("Before reallocation: \n%s\n", hello);

	// reallocating memory
	hello = realloc(hello, strlen("Hello World!\n") + 1);
	strncat(hello, " World!\n", strlen(" World!\n") + 1);
	printf("After reallocation: \n%s\n", hello);

	return 0;
}
