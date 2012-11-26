%option reentrant
%option prefix="action_"
%option bison-bridge
%option bison-locations
	/* %option noyywrap */
%option nounput
%option yylineno

%{
	#define USE_PACKET_TYPE_ENUM
	#include "action_context.h"
	#include "value_t.h"
	#include "action.tab.h"
	#ifdef  YY_EXTRA_TYPE
	# undef YY_EXTRA_TYPE
	#endif
	#define YY_EXTRA_TYPE action_context*
	#define YY_USER_ACTION yylloc->first_line = yylineno;

	#define YY_INPUT(buf,result,max_size) \
	{ \
		char c; \
		(yyextra->mutex).lock(); \
		(*yyextra->is) >> c; \
		if(yyextra->is->eof() || yyextra->wantexit>0 ) \
			result = YY_NULL; \
		else { \
			yyextra->os << c; \
			buf[0] = c; \
			result = 1; \
		} \
		(yyextra->mutex).unlock(); \
	}

	#include <stdio.h>
%}

%x ESC
%x QUOT
%x QUOTESC

	/* Definitions */
ABONAME	("/"[a-zA-Z0-9_]+)+
NAME	[a-zA-Z][a-zA-Z0-9_]*
INTEG	[0-9]+|"0x"[0-9a-fA-F]+
DOUBL	([0-9]+"."[0-9]*)|([0-9]*"."[0-9]+)

%%

	/* * * *  ESCAPES  * * * */
"{"			yyextra->esc_depth = 1; BEGIN(ESC);
\"			BEGIN(QUOT);
"//".*\n		/* comments ignored */
"#".*\n		/* comments ignored */


	/* * * *  TOKENS / KEYWORDS  * * * */

		/* Commands */
"ls"			return T_LIST;
"list"			return T_LIST;
"show"			return T_SHOW;
"load"			return T_LOAD;
"exit"			return T_EXIT;
"isset"			return T_ISSET;
"isdef"			return T_ISDEF;
"isndef"		return T_ISNDEF;
"echo"			return T_ECHO;
"sleep"			return T_SLEEP;
"if"			return T_IF;
"else"			return T_ELSE;
"while"			return T_WHILE;
"break"			return T_BREAK;
"do"			return T_DO;
"setid"			return T_SETID;
"waitfor"		return T_WAITFOR;
"pcall"			return T_PCALL;

"subscribe"		return T_SUBSCRIBE;
"unsubscribe"		return T_UNSUBSCRIBE;
"send"			return T_SEND;
"on"			return T_ON;

		/* Objects */
"new"			return T_NEW;
"packet_type"		return T_PACKET_TYPE;
"DATA"			{ yylval->Integer = PKT_DATA; return PACKET_TYPE; }
"SETDATA"		{ yylval->Integer = PKT_SETDATA; return PACKET_TYPE; }
"MANAGEMENT"		{ yylval->Integer = PKT_MANAGEMENT; return PACKET_TYPE; }

"proc"			return T_PROCEDURE;
"procedure"		return T_PROCEDURE;
"procedures"		return T_PROCEDURES;
"vars"			return T_VARS;
"types"			return T_TYPES;
"abos"			return T_ABOS;
"procs"			return T_PROCS;
"pos"			return T_POS;
"tree"			return T_TREE;
"name"			return T_NAME;

		/* data types */
"const"			return T_CONST;
"real"|"double"		return T_REAL;
"int"|"integer"		return T_INTEGER;
"string"		return T_STRING;
"unit"			return T_UNIT;


	/* * * *  VALUES * * * */

{INTEG}			{ yylval->Integer = strtol(yytext, (char**)NULL, 0); return INTEGER; }
{DOUBL}			{ yylval->Double = strtod(yytext, (char**)NULL); return REAL; }
{ABONAME}		{ yylval->cptr = strndup(yytext, yyleng); return ABONAME; }
"true"			{ yylval->Bool = true; return BOOL; }
"false"			{ yylval->Bool = false; return BOOL; }
"=="			return T_EQ_EQ;
">="			return T_GREATER_EQ;
"<="			return T_LESS_EQ;
"!="			return T_NOT_EQ;
"+="			return T_PLUS_EQ;
"-="			return T_MINUS_EQ;
"*="			return T_MUL_EQ;
"/="			return T_DIV_EQ;
"%="			return T_MOD_EQ;
"++"			return T_PLUS_PLUS;
"--"			return T_MINUS_MINUS;
{NAME}			yylval->cptr = strndup(yytext, yyleng); return NAME;

	/* * * *  CHARS & UNMACHED  * * * */

	/*;			return ';';*/

" "			/* ignore */
\n			/* ignore */
\t			/* ignore */
.			{ return yytext[0]; }


	/* * * *  INSIDE DOUBLE QUOTES  * * * */

<QUOT>\"	{
			yylval->cptr = strndup(yytext, yyleng-1);
		//	printf("Quoted finish \"%s\" length %d\n", yylval->cptr, yyleng-1);
			BEGIN(INITIAL);
			return QUOTED;
		}
<QUOT>"\\"	{ BEGIN(QUOTESC); yymore(); /*discard escape char*/ }
		
<QUOT>.		{ yymore(); }

<QUOTESC>.	{
			/*switch (yytext[yyleng-1]) {
			case 'n': yytext[yyleng-1]= '\n'; break;	// replace control chars
			case 't': yytext[yyleng-1]= '\t'; break;
			case 'r': yytext[yyleng-1]= '\r'; break;
			default: ;					// use unbackslashed char as default ('"', '\'...)
			}*/
			//yytext[yyleng-2] = yytext[yyleng-1];
			//yytext[--yyleng]=0;
			yymore();
			BEGIN(QUOT); 
		}

	/* * * *  INSIDE ESCAPED CODE  * * * */

<ESC>"}"	{
			yyextra->esc_depth--;
			if(yyextra->esc_depth == 0) {
				yylval->cptr = strndup(yytext, yyleng-1); 
				BEGIN(INITIAL); 
				return ESCAPE;
			} else {
				yymore();
			}
		}
<ESC>"{"	{
			yymore();
			yyextra->esc_depth++;
		}
<ESC>.		{	// non white-space chars
			yymore();
		}
<ESC>[\n\t ]+	{	// white-space chars
			yymore();
		}

%%


//#ifdef action_wrap
//#	undef action_wrap
//#endif
inline int action_wrap(void* vp) {
//	action_context* context = (action_context*)vp;
//	return context->pop_yyin();
	return 1;
}


void action_context::init_scanner() throw()
{
	yylex_init(&scanner);
	yyset_extra(this, scanner);
}

void action_context::destroy_scanner() throw()
{
	yylex_destroy(scanner);
}
