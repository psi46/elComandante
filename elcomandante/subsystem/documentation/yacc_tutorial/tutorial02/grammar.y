%{
#include <stdio.h>
#include <string.h>
 
void yyerror(const char *str) {
	fprintf(stderr,"error: %s\n",str);
}

int yywrap() {
	return 1;
} 

main() {
	printf("Type commands:\n");
	yyparse();
} 

%}

%token INTEGER FLOAT TSET TOUTPUT TGAIN TPOLARITY TOFFSET TDIGITAL NL
%%

commands: /* empty */
	| commands command
	;

command:
	set_output
	|
	set_gain
	|
	set_polarity
	|
	set_offset
	|
	set_digital
	|
	NL
	;

/*
set_command:
	TSET WHAT NUMBER NUMBER
	{
	switch ($2) {
	case TOUTPUT:   printf("\tChannel %d set to %d\n", $3, $4); break;
	case TGAIN:     printf("\tGain of channel %d set to %d\n", $3, $4); break;
	case TPOLARITY: printf("\tPolarity of channel %d set to %d\n", $3, $4); break;
	case TOFFSET:   printf("\tOffset of channel %d set to %d\n", $3, $4); break;
	case TDIGITAL:  printf("\tChannels 0x%X set to 0x%X\n", $3, $4); break;
	default:
		return 1;
	}
	;
*/

set_output:
	TSET TOUTPUT INTEGER FLOAT NL
	{
		printf("\tChannel %d set to %d (float)\n", $3, $4);
	}
	|
	TSET TOUTPUT INTEGER INTEGER NL
	{
		printf("\tChannel %d set to %d (int)\n", $3, $4);
	}
	;

set_gain:
	TSET TGAIN INTEGER FLOAT NL
	{
		printf("\tGain of channel %d set to %d\n", $3, $4);
	}
	;

set_polarity:
	TSET TPOLARITY INTEGER FLOAT NL
	{
		printf("\tPolarity of channel %d set to %d\n", $3, $4);
	}
	;

set_offset:
	TSET TOFFSET INTEGER FLOAT NL
	{
		printf("\tOffset of channel %d set to %d\n", $3, $4);
	}
	;

set_digital:
	TSET TDIGITAL INTEGER INTEGER NL
	{
		printf("\tChannels 0x%X set to 0x%X\n", $3, $4);
	}
	;
%%


