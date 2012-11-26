%locations
%defines
%error-verbose

%union
{
	int	Integer;
	char*	cstr;
}

%{
	#include <stdio.h>
	#include <string.h>
	#include <errno.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <fcntl.h>

	extern int showcode;
	void yyerror(const char *str) {
		fprintf(stderr,"error: %s\n",str);
	}

	int yywrap() {
		return 1;
	}

	extern FILE* yyin;
	int main(int argc, char* argv[]) {
		showcode=0;
		char** arg = argv;
		char* infile=NULL;
		int fd;
		while (arg++ < argv+argc-1 ) {
			if ( strcmp("-c", *arg)==0 ) { showcode++; }
			else { infile = *arg; };
		}

		if (infile != NULL) {	// if input file given redirect it to stdin
			//printf("input file: %s\n", infile);
			if ( (fd = open(infile, O_RDONLY)) < 0 ) {
				fprintf(stderr, "could not open file %s: %s\n", infile, strerror(errno));
				return -1;
			}
			//printf("opened as fd %d\n", fd);
			if ( (fd=dup2(fd,STDIN_FILENO)) < 0 ) {
				fprintf(stderr, "could redirect file to stdin: %s\n", strerror(errno));
				return -1;
			}
			//printf("dup'ed to fd %d\n", fd);
		}
		return yyparse();
	} // end main

%}
//%type <cstr> WORD
%token <cstr> NAME
%token <cstr> CODEBLOCK
%token <cstr> QUOTED
%token <cstr> DQUOTED
%token T_PP
%token T_END
%token T_BEGIN

%%


start:	T_BEGIN RULES T_END
	{
		printf("SYNTAX OKAY\n");
		YYACCEPT;
	}
	; /* SYNTAX T_END
	{
		printf("SYNTAX OKAY\n");
	}
	;
	*/
RULES:  /* */
	| RULES STMT
	;

STMT:	WORD ':' NONTERMS_LIST ';' {
		printf("\n");
	}
	;

NONTERMS_LIST:	NONTERMS
	| NONTERMS_LIST '|' NONTERMS
	;

NONTERMS: /* */
	| NONTERMS WORD
	| NONTERMS CODEBLOCK
	;

WORD:	NAME
	{
		printf("[%s]", $1);
	}
	| QUOTED
	{
		printf("'%s'", $1);
	}
	;

/*
TEXT:	empty/
	|
	TEXT WORD
	;
*/
%%
