#include <stdio.h>
#include <unistd.h>

#define IMAX	10
#define LINELEN	80

int main(void) {
	int i=0;
	char line[LINELEN];
	while (i<IMAX) {
		if ( fgets(line, LINELEN, stdin) == NULL ) break;
		printf("read line %d: \"%s\" (stdout)\n", i, line); fflush(stdout);
		i++;
	}
	printf("loop exited, after %d lines. (max: %d) (stdout)\n", i, IMAX); fflush(stdout);
	return 0;
}
