#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	char ** a = malloc(sizeof(char *) * 10);
	for (int i = 0; i < 10; i++) {
		a[i] = malloc(sizeof(char) * 10);
		strcpy(a[i], i % 2 == 0 ? "even" : "odd");
	}

	// contiguous because operating system is allocating nearby memory addresses in the heap; likewise there is additional padding hence the extra byte difference over 10.
	// every variable has a memory address, don't confuse the address of the variable and the variable containing another memory address as its value.
	printf("Variable Location (address): %p\n", &a); // stack memory address
	// this is the true value of the addresses of addresses your c-strings lives at (the value that your variable holds).
	printf("Double Pointer Value (address): %p\n", a); // heap memory address
	printf("Pointer to first character of first c-string: %p\n", *a); // heap memory address
	for (int i = 0; i < 10; i++) {
		printf("[%d]: Pointer to first character of %d cstring: %p, Location: %p, String: %s\n", i, i, a[i], &a[i], a[i]);
	}

	char * c = malloc(sizeof(char) * 10);
	// make sure to clear the memory before assigning a new value to it.
	free(a[5]);
	strcpy(c, "hello");
	// we are assigning a memory address to a[5] not a value at a[5]! a[5] has a memory address that points to the value "hello" in our heap!
	a[5] = c;

	printf("\nAfter free(a[5]) and assignment of new value to a[5].\n");

	// notice how memory addresses aren't contiguous.
	// we are reading 8 bytes (64 bit) at a time and those 8 bytes point to memory addresses that contain the start of a c-string.
	for (int i = 0; i < 10; i++) {
		printf("[%d]: Pointer to first character of %d cstring: %p, Location: %p, String: %s\n", i, i, a[i], &a[i], a[i]);
	}

	// likewise, a[5] = c, is equivalent to this:
	printf("a[5] = %s\n", *(a + 5)); // you should avoid this notation, it's confusing; use a[i] instead whenever you can.
	// likewise, we can dereference again to get individual characters.
	printf("a[5][1]'s second character = %c, or this: %c. \n", a[5][1], *(*(a + 5)+ 1));

	// let's try changing the first c-string to a different value.
	char * d = malloc(sizeof(char) * 10);
	free(a[0]);
	strcpy(d, "world");
	a[0] = d;

	printf("\nAfter free(a[0]) and assignment of new value to a[0].\n");

	// notice how memory addresses stored as values aren't contiguous, but the location still is!
	for (int i = 0; i < 10; i++) {
		printf("[%d]: Pointer to first character of %d cstring: %p, Location: %p, String: %s\n", i, i, a[i], &a[i], a[i]);
	}

	for(int i = 0; i < 10; i++) {
		free(a[i]);
	}

	free(a);
	return 0;
}
