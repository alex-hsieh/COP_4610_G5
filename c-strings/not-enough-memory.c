#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // Allocate memory for our buffer.
    char *buffer = (char*) malloc(1);

    // Intentionally overflow the buffer.
	// strcpy(buffer, "This is a long string that will overflow the buffer.\n");
	// strcat(buffer, "This is a long string that will overflow the buffer.\n");
	strncpy(buffer, "This is a long string that will overflow the buffer.\n", strlen("This is a long string that will overflow the buffer.\n") + 1);
	strncat(buffer, "This is a long string that will overflow the buffer.\n", strlen("This is a long string that will overflow the buffer.\n") + 1);

	printf("%s", buffer);
    free(buffer);
    return 0;
}
