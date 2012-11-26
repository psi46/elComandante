%{
#include <stdio.h>
#include "flex_sublpt_gen.tab.h"
#include "lpt.h"
%}
%%
[0-9]+		yylval=atoi(yytext); return NUMBER;
0x[0-9A-Fa-f]+	yylval=atoi(yytext); return NUMBER;
set		return T_SET;
on|off		yylval=!strcmp(yytext, "on"); return STATE;
gain		return T_GAIN;
output		return T_OUTPUT;
digital		return T_DIGITAL;
polarity	return T_POLARITY;
offset		return T_OFFSET;
exit		return T_EXIT;
\n		/* ignore end of line */;
[ \t]+		/* ignore whitespace */;
.		return *yytext;
%%

