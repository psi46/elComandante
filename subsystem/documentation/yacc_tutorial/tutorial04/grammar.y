%{
#include <stdio.h>
#include <string.h>

// GLOBALS
int yydebug=0;
int wantexit=0;

void yyerror(const char *str) {
	fprintf(stderr,"parser error: %s\n",str);
}

int yywrap() {
	return 1;
} 

int main(void) {
	wantexit=0;
	while (!wantexit) {
		printf("> "); fflush(stdout);
		yyparse();
	}
	printf("bye.\n");
	return 0;
} 

%}

%start command
%union {
	int integer;
	double real;
	char* words;
}
%token <integer> INTEGER
%token <real>	 REAL
%type  <real>	 VALUE
%token <integer> TDEBUG
%token <words>	 TUNKNOWN
%token TSET TOUTPUT TGAIN TPOLARITY TOFFSET TDIGITAL NL TEXIT TALL

%%

/*commands: *//* empty */
/*	| commands command
	;*/

command:  set_output	{ YYACCEPT; }
	| set_gain	{ YYACCEPT; }
	| set_polarity	{ YYACCEPT; }
	| set_offset	{ YYACCEPT; }
	| set_digital	{ YYACCEPT; }
	| NL		{ YYACCEPT; }
	| debug_switch	{ YYACCEPT; }
	| TEXIT NL	{ YYACCEPT; }
	| TUNKNOWN	{ YYABORT; }

debug_switch: TDEBUG NL
	{
		yydebug=$1;
		printf("debuging turned %s\n", ($1?"on":"off"));
	}

/*
 * I don't know jet, if this would be a better solution:
 *
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

VALUE:	  REAL { $$=$1; }
	| INTEGER { $$=(double)$1; }

set_output:
	TSET TOUTPUT INTEGER VALUE NL
	{
		printf("\tChannel %d set to %lf\n", $3, $4);
	}
	;

set_gain:
	TSET TGAIN INTEGER VALUE NL
	{
		printf("\tGain of channel %d set to %lf\n", $3, $4);
	}
	;

set_polarity:
	TSET TPOLARITY INTEGER VALUE NL
	{
		printf("\tPolarity of channel %d set to %lf\n", $3, $4);
	}
	;

set_offset:
	TSET TOFFSET INTEGER VALUE NL
	{
		printf("\tOffset of channel %d set to %lf\n", $3, $4);
	}
	;

set_digital:
	TSET TDIGITAL INTEGER INTEGER NL
	{
		printf("\tChannels 0x%X set to 0x%X\n", $3, $4);
	}
	;
%%


