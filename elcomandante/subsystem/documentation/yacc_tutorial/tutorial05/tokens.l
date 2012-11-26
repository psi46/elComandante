%{
#include <stdio.h>
#include <stdlib.h>
#include "grammar.tab.h"	// Make compiler aware of grammars tokens

extern int yydebug;
extern int wantexit;

/* This function is necessary to read lines from a pipe.
 * It replaces the stdlib::isatty() function to fake a
 * terminal for the parser. */
int isatty(int x) { return 1; }

%}

%%

[0-9]+|0x[0-9a-fA-F]+   { if (yydebug) printf(" INT<%s>", yytext); yylval.integer=strtol(yytext, (char**)NULL, 0); return INTEGER; }
([0-9]*\.[0-9]+)|([0-9]+\.[0-9]*) {
		if (yydebug) printf(" REAL<%s>", yytext);
		yylval.real=strtod(yytext, (char**)NULL); return REAL;
	}
set|SET                 { if (yydebug) printf(" TSET");	return TSET; }
output|OUTPUT		{ if (yydebug) printf(" TOUTPUT");	return TOUTPUT;}
gain|GAIN               { if (yydebug) printf(" TGAIN");	return TGAIN;}
polarity|POLARITY   	{ if (yydebug) printf(" TPOLARITY");	return TPOLARITY;}
digital|DIGITAL		{ if (yydebug) printf(" TDIGITAL");	return TDIGITAL;}
\n                      { if (yydebug) printf(" NL\n");	return NL;}
debug			{ printf(" TDEBUG<1>"); yylval.integer=(1==1); return TDEBUG; }
nodebug			{ yylval.integer=(1==0); return TDEBUG; }
exit			{ wantexit++; return TEXIT; }
all|ALL			{ return TALL; }
[ \t]+                  /* ignore whitespace */;
.	 return (int)yytext[0];

%%

//.+			{ printf("unknown token: %s\n", yytext); yylval.words=strdup(yytext); return TUNKNOWN; }

