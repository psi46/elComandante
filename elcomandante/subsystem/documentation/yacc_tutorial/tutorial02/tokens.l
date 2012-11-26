%{
#include <stdio.h>
#include <stdlib.h>
#include "grammar.tab.h"	// Make compiler aware of grammars tokens
%}

%%

[0-9]+|0x[0-9a-fA-F]+   printf(" INT"); yylval=strtol(yytext, (char**)NULL, 0); return INTEGER;
([0-9]*\.[0-9]+)|([0-9]+\.[0-9]*) printf(" FLOAT"); yylval=atoi(yytext); return FLOAT;/*			yylval=strtod(yytext); return FLOAT;*/
set|SET                 printf(" TSET");	return TSET;
output|OUTPUT		printf(" TOUTPUT");	return TOUTPUT;
gain|GAIN               printf(" TGAIN");	return TGAIN;
polarity|POLARITY   	printf(" TPOLARITY");	return TPOLARITY;
digital|DIGITAL		printf(" TDIGITAL");	return TDIGITAL;
\n                      printf(" NL\n");	return NL;
[ \t]+                  /* ignore whitespace */;
.	return (int)yytext[0];

%%
