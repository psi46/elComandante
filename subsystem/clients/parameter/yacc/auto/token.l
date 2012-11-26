

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grammar.tab.h"	// Make compiler aware of grammars tokens

extern int yydebug;
extern "C" {
	int yywrap() { return 1; }
}
extern int wantexit;
%}

%%

[0-9]+|0x[0-9a-fA-F]+   { if (yydebug) printf(" INT<%s>", yytext); yylval.integer=strtol(yytext, (char**)NULL, 0); return INTEGER; }
([0-9]*\.[0-9]+)|([0-9]+\.[0-9]*) {
		if (yydebug) printf(" REAL<%s>", yytext);
		yylval.real=strtod(yytext, (char**)NULL); return REAL;
	}
set|SET                 { return TSET; }
voltage|VOLTAGE		{ return TVOLTAGE;}
current|CURRENT         { return TCURRENT;}
all|ALL			{ return TALL; }

\n                      { return NL;}
on|ON			{ return TON; }
off|OFF			{ return TOFF; }
option|OPTION		{ return TOPTION; }
[kK][iI][lL]+		{ return TKILL; }

exit			{ wantexit++; return TEXIT; }
[ \t]+                  /* ignore whitespace */;
[a-zA-Z0-9_]+		{ yylval.words=strdup(yytext); return TWORD;}

%%

//.+			{ printf("unknown token: %s\n", yytext); yylval.words=strdup(yytext); return TUNKNOWN; }

