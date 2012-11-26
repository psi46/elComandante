/**
 * \file subscript.l
 * \author Dennis Terhorst
 * \date Sun Jul 19 15:27:27 CEST 2009
 */

/* configure flex */
%option reentrant
%option prefix="subscript_"
%option bison-bridge
%option bison-locations
%option noyywrap
%option yylineno
%option interactive
%option stack

%{
	#include <stdio.h>
	#include "action_context_t.h"	// context definition
	//#include "subscript.h"		// load YYSTYPE def
	#include "subscript.tab.h"	// load grammars tokens

	#define YY_EXTRA_TYPE action_context_t*
	#define YY_USER_ACTION yylloc->first_line = yylineno;
	#define thiscontext	yyextra
	#define YY_INPUT(buf,result,max_size) {	\
		char c;				\
		(*thiscontext->is) >> noskipws >> c;		\
		if(thiscontext->is->eof())		\
			result = YY_NULL;	\
		else {				\
			buf[0] = c;		\
			result = 1;		\
		}				\
	}

	//extern int yydebug;
	//extern "C" {
	//	int yywrap() { return 1; }
	//}
%}

/* We define an exclusive scanner state ESC to deal with escaping */
%x ESC
%x QSTRING
%%

	/* VALUES */
[0-9]+|0x[0-9a-fA-F]+   { yylval->integer=strtol(yytext, (char**)NULL, 0); return INTEGER; }
([0-9]*\.[0-9]+)|([0-9]+\.[0-9]*) { yylval->real=strtod(yytext, (char**)NULL); return REAL; }

	/* KEYWORDS */
new			{ return T_NEW; }
on			{ return T_ON; }
action			{ return T_ACTION; }
if			{ return T_IF; }
define			{ return T_DEFINE; }
packet			{ return T_PACKET; }
list			{ return T_LIST; }
done|exit		{ return T_DONE; }
echo			{ return T_ECHO; }
double			{ return T_DOUBLE; }
integer			{ return T_INTEGER; }
string			{ return T_STRING; }
in			{ return T_IN; }
nounit			{ return T_NOUNIT; }
"{"			{
			  thiscontext->esc_depth = 1;
			  //(thiscontext->is) >> noskipws;
			  BEGIN(ESC);
			}
\"			{ BEGIN(QSTRING); }
[ \t\n]+		/* ignore whitespace */;
";"			{ return ';'; }
[a-zA-Z_][0-9a-zA-Z_]*	{ yylval->cptr = strndup(yytext, yyleng); return IDENTIFIER; }
.			{ return (int)yytext[0]; }

	/* ESCAPED STATES */
<ESC>"}"		{
			  thiscontext->esc_depth--;
			  if(thiscontext->esc_depth == 0) {
				yylval->cptr = strndup(yytext, yyleng-1); 
				BEGIN(INITIAL); 
				return CODE_BLOCK;
			  } else {
				yymore();
			  }
			}
<ESC>"{"		{ yymore(); thiscontext->esc_depth++; }
<ESC>[^}]*		{ yymore(); }

	/* WSTRING */
<QSTRING>\"		{
			  yylval->cptr = strndup(yytext, yyleng-1); 
			  BEGIN(INITIAL); 
			  return QUOTED_STRING;
			}
<QSTRING>[^"]*		{ yymore(); }

%%

// implementation of the scanner functions in action_context_t
void action_context_t::init_scanner() {
	yylex_init(&scanner);
	yyset_extra(this, scanner);
}

void action_context_t::destroy_scanner() {
	yylex_destroy(scanner);
}

