#include <stdio.h>
#include <unistd.h>

#define IMAX	10

int main(void) {
	int i=0;
	while (i<IMAX) {
		printf("i=%d (stdout)\n", i); fflush(stdout);
		sleep(1);
		i++;
	}
	printf("i reached IMAX=%d, i will stop now. (stdout)\n", IMAX);
	return 0;
}
