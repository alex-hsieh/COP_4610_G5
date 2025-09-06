#include <stdio.h>
#include <stdlib.h>

int main() {
	char * x = 0; // this is the zero address, null address, we aren't supposed to set a value to this.
	// segfault because we are trying to access memory that we don't have access to.
	printf("%p\n", x); // as expected, we get zero address printed.
	// printf("%p", x); // additionally, know that you must flush your buffer before a segfault with newline or fflush(stdout), otherwise you won't get any message.
	*x = 'a';
	return 0;
}
