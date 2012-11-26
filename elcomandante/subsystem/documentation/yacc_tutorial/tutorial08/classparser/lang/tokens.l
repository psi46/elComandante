
%{
#include "grammar.tab.h"	// Make compiler aware of grammars tokens
extern YYSTYPE yylval;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int esc_depth=0;
int showcode=0;
%}

%x CODE
%x PART
%x QUOT
%x DQUOT
%x QUOTESC
%x COMMENT


%%

"%%\n"		{
			BEGIN(PART); 
			//printf("\n");
			return T_BEGIN;
		}
.		{	/*printf(".");  skip all */ };
[ \n\t]+	{	/*printf(".");  skip all */ };


	/*   P A R T 2    */

<PART>"{"	{
			if (showcode) printf("\t<CODE");
			esc_depth = 1;
			BEGIN(CODE);
		}
<PART>\'	{	BEGIN(QUOT);   }
<PART>"//"[^\n]* {	//printf("<COMM");
		}

<PART>"/*"	{	//printf("<COMM");
			BEGIN(COMMENT);	}

<PART>"%%"	{
			BEGIN(INITIAL);
			return T_END;
		}


		/*"{"		{ esc_depth=1; BEGIN(CODE); }*/

		/*^"%"[^%{}]*	{ /* ignore directive }
		^"%{"		{ esc_depth=1; BEGIN(CODE); } */

<PART>"\n"+|" "+|"\t"+		{ printf("%c", yytext[0]); /* ignore */ }

<PART>[^ "'{\n\t|:;]+		{
			//printf("<%s>", yytext);
			yylval.cstr = strndup(yytext, yyleng);
			return NAME;
			}

<PART>";"		{ printf("%c", yytext[0]); return yytext[0]; }
<PART>":"[ \t\n]+	{ printf(" %c ", yytext[0]); return yytext[0]; }
<PART>"|"		{ printf("-|-"); return yytext[0]; }

<PART>.		{ printf("unmatched: '%s'\n", yytext); }


	/* * * *  INSIDE ESCAPED CODE  * * * */

<CODE>"}""\n"*	{
		  if (--esc_depth == 0 ) {
			//yylval->cstr = strndup(yytext, yyleng-1);
			if (showcode) printf("BLOCK>\n");
			BEGIN(PART);
			return CODEBLOCK;
		  } else {
			yymore();
		  }
		}
<CODE>"{"	{
			yymore();
			esc_depth++;
		}
<CODE>[\n\t ]+	{ yymore(); }
<CODE>.		{ yymore(); }



	/* * * *  INSIDE COMMENTS  * * * */

<COMMENT>"*/"	{
			//yylval->cptr = strndup(yytext, yyleng-1);
			//printf("ENT>");
			BEGIN(PART);
			// ignore	
		}
<COMMENT>.		{ yymore(); }
<COMMENT>[\n\t ]+	{ yymore(); }

	/* * * *  INSIDE SINGLE QUOTES  * * * */

<QUOT>"'"		{
			yylval.cstr = strndup(yytext, yyleng-1);
			BEGIN(PART);
			return QUOTED;
		}
<QUOT>.		yymore();

	/* * * *  INSIDE DOUBLE QUOTES  * * * */

<DQUOT>\"	{
			yylval.cstr = strndup(yytext, yyleng-1);
			BEGIN(PART);
			return DQUOTED;
		}
<DQUOT>"\\"	BEGIN(QUOTESC); yymore();
		
<DQUOT>.		yymore();

<QUOTESC>.	{
			switch (yytext[yyleng-1]) {
			case 'n': yytext[yyleng-1]= '\n'; break;	// replace control chars
			case 't': yytext[yyleng-1]= '\t'; break;
			case 'r': yytext[yyleng-1]= '\r'; break;
			default: ;					// use unbackslashed char as default ('"', '\'...)
			}
			yytext[yyleng-2] = yytext[yyleng-1];
			yytext[--yyleng]=0;
			yymore();
			BEGIN(DQUOT); 
		}


%%
