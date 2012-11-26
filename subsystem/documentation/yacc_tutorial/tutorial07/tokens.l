%{
#include <stdio.h>
#include <stdlib.h>
#include "grammar.tab.h"	// Make compiler aware of grammars tokens

extern int yydebug;
extern "C" {
	int yywrap() { return 1; }
}

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
[ \t]+                  /* ignore whitespace */;
.	return (int)yytext[0];

%%
