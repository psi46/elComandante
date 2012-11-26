/**
 * \file subscript.y
 * \author Dennis Terhorst
 * \date Sun Jul 19 15:36:50 CEST 2009
 */

// configure bison
%pure-parser
%name-prefix="subscript_"
%locations
%defines
%error-verbose
%parse-param { action_context_t* context }
%lex-param { void* scanner  }

// TOKEN DEFINITIONS

%start start

// %code pre
//%{
//	#include "sensations.h"
//	#include "subscript.h"
//%}

%union {
	int integer;
	double real;
	char* cptr;
}
//	sens_vec_t* senvecptr;
//	sens_value_t* sensval;	

/* VALUED TOKENS */
%token <integer> INTEGER

%token <real>	 REAL

%token <cptr>	CODE_BLOCK
%destructor { free($$); } CODE_BLOCK

%token <cptr>	QUOTED_STRING
%destructor { free($$); } QUOTED_STRING
//%type <cptr>	UNIT

%token <cptr>	IDENTIFIER
%destructor { free($$); } IDENTIFIER

//%type	<sensval> SENSVAL
//%destructor { delete($$); } SENSVAL

/* KEYWORDS */
%token T_NEW T_ON T_ACTION T_IF T_DEFINE T_PACKET T_LIST T_DONE T_ECHO
%token T_DOUBLE T_INTEGER T_STRING T_IN T_NOUNIT

// Note that this C code is defined after the definition of the %union, which
// means that this code will go into the C++ file (LanAB.tab.c) instead of the
// header file (LanAB.tab.h).

// for discussion see
// http://www.gnu.org/software/bison/manual/html_node/Prologue-Alternatives.html
//%code {
%{
	#include "action_context_t.h"
	#include <iostream>
	#include <sstream>
	using namespace std;

	//int yylex();
	int subscript_lex(YYSTYPE* lvalp, YYLTYPE* llocp, void* scanner);

	void subscript_error(YYLTYPE* locp, action_context_t* context, const char* err) {
		cout << context->FullName() <<":"<< locp->first_line << ": " << err << endl;
	}

	//extern "C" {
	//	void yyerror(const char *str) { fprintf(stderr,"error: %s\n",str); }
	//}
	#define scanner context->scanner
%}



%%

start:	STATEMENTS

STATEMENTS: /* empty */
	| STATEMENTS FULL_STATEMENT 

FULL_STATEMENT:	STATEMENT  { cout << context->FullName() << "> " << flush; }

STATEMENT:	  T_NEW T_ACTION CODE_BLOCK ';' {
		cout << "NEW ACTION {" << $3 << "}" << endl;
	}
/*	|
	DEFINE_PACKET {
		printf("DEFINE PACKET\n");
	}*/
	| T_ON ';' {
		cout << "ONCLAUSE" << endl;
	}
	| T_LIST T_PACKET ';' {
		context->list_packet_definitions();
	}
	| T_LIST T_ACTION ';' {
		cout << "LIST OF ACTIONS:" << endl;
		//AC->list_actions();
		cout << "0 total" << endl;
	}
	| IDENTIFIER ';' {
		cout << "IDENTIFIER " << $1 << endl;
	}
	| T_ECHO QUOTED_STRING ';' {
		cout << $2 << endl;
	}
	| T_DONE {
		cout << "DONE" << endl;
		YYACCEPT;
	}
	| ';' {
		/* ignore empty statement */
	}
	;
/*
SENS_LIST:SENSVAL {
		
	}
	| SENS_LIST ',' SENSVAL {
		cout << $3;
	}

SENSVAL:  T_DOUBLE IDENTIFIER T_IN UNIT {
		$$ = new sens_value_t($2, new valued_t($4) );
	}
	| T_DOUBLE IDENTIFIER T_IN UNIT T_NOUNIT {
		$$ = new sens_value_t($2, new valuei_t($4, value_t::NO_READ_UNIT) );
	}

UNIT:	QUOTED_STRING { $$=$1; }
*/
/*
NEW_ACTION:	T_NEW T_ACTION CODE_BLOCK ID
	{ current_context.add(new action($ID, $CODE_BLOCK)); }
DEFINE_PACKET:	T_DEFINE T_PACKET '"' ABONAME '"' PKT_TYPE SENS_LIST
	{ current_context.add(new packet_description($ABONAME, PKT_TYPE, ) ); }

ONCLAUSE:	T_ON SENS_EXPR ':' ID
	{ current_context.add(new on_clause() ) }

SENS_EXPR:	SENS
	|	'(' SENS_EXPR ')'
	|	SENS_EXPR '+' SENS_EXPR { $$= new sens_add_t($1, $3)}
	|	SENS_EXPR '-' SENS_EXPR { $$= new sens_sub_t($1, $3)}
	|	SENS_EXPR '*' SENS_EXPR { $$= new sens_mul_t($1, $3)}
	|	SENS_EXPR '/' SENS_EXPR { $$= new sens_div_t($1, $3)}
*/
%%

