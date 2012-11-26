#include <stdio.h>
#include <unistd.h>

#define IMAX	100

int main(void) {
	int i=0;
	while (i<IMAX) {
		fprintf(stderr, "i=%d (stderr)\n", i);
		sleep(1);
		i++;
	}
	fprintf(stderr, "i reached IMAX=%d, i will stop now. (stderr)\n", IMAX);
	return 0;
}
