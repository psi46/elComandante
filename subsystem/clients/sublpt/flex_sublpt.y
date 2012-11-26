
%{
#include <stdio.h>
#include <string.h>

void yyerror(const char *str);
int yywrap();
int main(int argc, char* argv[]);

int wantexit=0;

%}

%token T_EXIT;
%token NUMBER T_SET STATE;
%token T_GAIN T_OUTPUT T_DIGITAL T_POLARITY T_OFFSET;

%%

commands: /* empty */
	| commands command
	;

command:
	set_output
	|set_gain
	|exit
	;

set_output:
	T_SET T_OUTPUT NUMBER NUMBER
	{
		printf("\tsetting channel %d to %d\n", $3, $4);
	}
	;

set_gain:
	T_SET T_GAIN NUMBER NUMBER
	{
		printf("\tCH%d: gain %d set\n", $3, $4);
	}
	;
exit:	T_EXIT { wantexit++; printf("exit at next break\n"); }
	;
%%


void yyerror(const char *str)
{
	fprintf(stderr,"yyerror: %s\n",str);
}

int yywrap()
{
	fprintf(stderr,"yywrap()\n");
	return 1;
} 


int main(int argc, char* argv[])
{
	printf("Hello World\n");
	int ret;
	while (wantexit==0) {
		ret = yyparse();
		printf("MAIN: yyparse() returned %d\n", ret);
		printf("      yytext = %s\n", yytext);
		printf("      yyval = %d\n", yyval);
	}

	printf("Bye World\n");
	return 0;
} 

