#include <stdio.h>
#include "parser.h"

extern int yydebug;

int main(void) {
	yydebug=0;
	printf("Hello World\n");
	if ( yyparse() ) {
		printf("File NOT ok.\n");
	} else {
		printf("File OK.\n");
	}
	return 0;
}

