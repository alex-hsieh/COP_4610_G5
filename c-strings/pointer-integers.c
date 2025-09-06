#include <stdlib.h>
#include <stdio.h>

int main() {
	// set aside 3 integers in memory. (4 * 3) = 12 bytes -- integer is 4 bytes (usually)
	int * a = (int *) malloc(sizeof(int) * 3);
	// assign them values at those locations
	a[0] = 1;
	a[1] = 2;
	a[2] = 3;
	for(int i = 0; i < 3; i++) {
		printf("value: %d, address: %p\n", a[i], &a[i]);
	}
	return 0;
}

