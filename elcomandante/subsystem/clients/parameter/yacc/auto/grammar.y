%{
#include <stdio.h>
#include <string.h>
#include "lpt.h"


// GLOBALS

int test1=0+42;
int test2=1+42;
double test3=2+42;
int test4=3+42;

int (*callback)(int,int);
int yydebug=0;
int wantexit=0;
int parfd=-1;
char bitpat=0;

extern int yylex();

void yyerror(const char *str) {
	fprintf(stderr,"parser error: %s\n",str);
}

int yywrap() {
	return 1;
} 

void __attribute__((constructor)) init() {
	printf("dl constructor: opening parport\n");
	parfd = lpt_open("/dev/parport0"); // OPEN
}

void __attribute__((destructor)) fini() {
	printf("dl destructor: closing parport\n");
	lpt_close(); //parfd); // CLOSE
}

/*int main(void) {
	wantexit=0;

	parfd = lpt_open("/dev/parport0"); // OPEN
	while (!wantexit) {
		printf("> "); fflush(stdout);
		yyparse(); // PARSE INPUT
	}
	printf("bye.\n");
	lpt_close(parfd); // CLOSE
	return 0;
} */

%}

%start command
%union {
	int integer;
	double real;
	char* words;
}


%token <integer> INTEGER

%token TTEST1 TTEST2 TTEST3 TTEST4 

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
	| auto_command	{ YYACCEPT; }
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
		char mask = (1<<$3);
		printf("\tChannel %d set to %lf\n", $3, $4);
		if ($4 > .5) {
			bitpat |= (1<<$3); // set bit
		} else {
			bitpat &= ~(1<<$3); // clear bit
		}
		lpt_setdata(parfd, bitpat);
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
auto_command:
	TSET TTEST1 VALUE NL
	  {
		printf("ACTION for test1\n");
		test1=$3;
	  }
	| TSET TTEST2 VALUE NL
	  {
		printf("ACTION for test2\n");
		test2=$3;
	  }
	| TSET TTEST3 VALUE NL
	  {
		printf("ACTION for test3\n");
		test3=$3;
	  }
	| TSET TTEST4 VALUE NL
	  {
		printf("ACTION for test4\n");
		test4=$3;
	  }
	  ;
%%


