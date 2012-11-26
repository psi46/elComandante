%{
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <error.h>

#define ALLCHAN -1

extern FILE* yyin;

extern "C" {
	void yyerror(const char *str) { 
		fprintf(stderr,"parser error: %s\n",str); 
	}
}
int yylex();

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
%token <words>	 TWORD
%type  <integer> channel

%token TSET TVOLTAGE TCURRENT NL TON TOFF TOPTION TKILL TEXIT TALL

%%

/*commands: *//* empty */
/*	| commands command
	;*/

command:  set_command	{ YYACCEPT; }
	| kill_command	{ YYACCEPT; }
	| on_command	{ YYACCEPT; }
	| off_command	{ YYACCEPT; }
	| NL		{ YYACCEPT; }
	| TEXIT NL	{ YYABORT; }
	| error NL	{ YYABORT; }

VALUE:	  REAL { $$=$1; }
	| INTEGER { $$=(double)$1; }

channel:
	  INTEGER { if ($1 < 0) { YYABORT; } else $$=$1; }
	| TALL { $$=ALLCHAN; }

set_command:
	  TSET TVOLTAGE channel VALUE NL
	  {
		printf("\tChannel %d set to %lf V\n", $3, $4);
	  }
	| TSET TCURRENT channel VALUE NL
	  {
		printf("\tChannel %d set to %lf uA\n", $3, $4);
	  }
	| TSET TON channel NL
	  {
		printf("\tChannel %d ON\n", $3);
	  }
	| TSET TOFF channel NL
	  {
		printf("\tChannel %d OFF\n", $3);
	  }
	| TSET TOPTION TWORD channel VALUE NL
	  {
		printf("\tChannel %d OPTION %s set to %lf\n", $4, $3, $5);
	  }
	;

kill_command:
	TKILL INTEGER NL
	  {
		printf("\tKILL CHANNEL %d\n", $2);
	  }
	| TKILL NL
	  {
		printf("\tKILL ALL\n");
	  }
	;

on_command:
	TON NL
	{
		printf("\tALL CHANNELS ON\n");
	}
	;
off_command:
	TOFF NL
	{
		printf("\tALL CHANNELS OFF\n");
	}
	;

%%

