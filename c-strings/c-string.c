#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main() {
	char * a = malloc(sizeof(char) * strlen("Hello world!")+1);
	strcpy(a, "Hello world!");

	printf("%s\n", a);

	// address of what the pointer points to.
	printf("value of pointer (address): %p\n", a); // heap memory
	// address of actual pointer
	printf("address of variable (address): %p\n", &a); // stack memory
	// first character of pointer
	printf("%c\n", *a);
	// printing entire string manually; look for null character to terminate!
	while (*a != '\0') {
		printf("character: '%c', address: %p\n", *a, a);
		a++;
	}
}
