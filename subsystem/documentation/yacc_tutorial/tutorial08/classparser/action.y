%pure-parser
%name-prefix="action_"
%locations
%defines
%error-verbose
%parse-param { action_context* context }
%lex-param { void* scanner  }

%{
#include "value_t.h"
//#include "sens_value_t.h"	// by value_t.h
#include "sens_arith_t.h"
//#include "sensations.h"		// by sens_value_t.h
#include "packet_description_t.h"

%}

%union
{
	int Integer;
	double Double;
	char* cptr;
	bool Bool;
	packet_description_t* pktdescptr;
	value_t* valueptr;
	sensation_t* sensptr;
	sens_value_t* sensvalptr;
	sens_list_t* senslistptr;
}

	/* * * *  Escapes  * * * */
%token <cptr> ESCAPE 
%token <cptr> QUOTED 


	/* * * *  TOKENS / KEYWORDS  * * * */

		/* Commands */
%token T_ERR
%token T_LIST
%token T_SHOW
%token T_LOAD 
%token T_EXIT
%token T_ISSET
%token T_ISDEF
%token T_ISNDEF
%token T_ECHO
%token T_SLEEP
%token T_IF
%token T_ELSE
%token T_WHILE
%token T_BREAK
%token T_DO
%token T_SETID
%token T_WAITFOR
%token T_PCALL

%token T_SUBSCRIBE
%token T_UNSUBSCRIBE
%token T_SEND
%token T_ON

		/* Objects */
%token T_NEW
%token T_PACKET_TYPE

%token T_ABOS T_VARS T_TYPES T_PROCS T_POS T_TREE
%token T_PROCEDURE
%token T_PROCEDURES
%token T_NAME

		/* data types */
%token T_CONST
%token T_REAL T_INTEGER T_STRING
%token T_UNIT


	/* * * *  VALUES  * * * */

%token <Integer> INTEGER
%token <Double> REAL
%token <Bool> BOOL
%token <cptr> ABONAME
%token <pktdescptr> PACKET_DESCRIPTION
%token <valueptr> VALUE
%token <Integer> PACKET_TYPE
%token T_TRUE T_FALSE
%token <cptr> NAME
%type <sensptr> EXPRESSION
%destructor { free ($$); } EXPRESSION

		/* operators */
// To avoid a lot of shift/reduce conflicts, the precedence of the operators
// must be defined. See bison manual, it also states the following:
// * The associativity of an operator op determines how repeated uses of the
//   operator nest: whether ‘x op y op z’ is parsed by grouping x with y first or
//   by grouping y with z first. %left specifies left-associativity (grouping x
//   with y first) and %right specifies right-associativity (grouping y with z
//   first). %nonassoc specifies no associativity, which means that ‘x op y op z’
//   is considered a syntax error.
// * The precedence of an operator determines how it nests with other
//   operators. All the tokens declared in a single precedence declaration have
//   equal precedence and nest together according to their associativity. When
//   two tokens declared in different precedence declarations associate, the one
//   declared later has the higher precedence and is grouped first. 

// see operators.txt
%left T_PLUS_EQ T_MINUS_EQ T_MUL_EQ T_DIV_EQ T_MOD_EQ '='
%left T_EQ_EQ T_NOT_EQ
%left T_GREATER_EQ T_LESS_EQ '>' '<'
%left '+' '-' '.'
%left '*' '/' '%'
%left T_PLUS_PLUS T_MINUS_MINUS


		/* non-terminal symbol types */
%type <Integer> actions
%type <Integer> action
%type <cptr> filename
%type <sensvalptr> SENSVALUE
%type <senslistptr> SENSLIST
%destructor { free ($$); } ESCAPE QUOTED NAME ABONAME
%destructor { free ($$); } PACKET_DESCRIPTION VALUE SENSVALUE
%destructor { $$->clear(); } SENSLIST


%{
	#include <iostream>
	#include <sstream>
	#include <fstream>
	#include "action_context.h"
	#include "sensations.h"

//	extern int yyerrstatus;
	// as defined in grammar.tab.c
//	#define yyerrok            (yyerrstatus = 0)

	using namespace std;

	int action_lex(YYSTYPE* lvalp, YYLTYPE* llocp, void* scanner);		

	void action_error(YYLTYPE* locp, action_context* context, const char* err)
	{
		cout << "l" << locp->first_line << "c" << locp->first_column << "-l" << locp->last_line << "c" << locp->last_column << ":" << err << endl;
		//yyerrok; FIXME: error reporting has to restart right away, not just after 3 further tokens...
		// (see http://www.gnu.org/software/bison/manual/html_node/Error-Recovery.html#Error-Recovery)
	}

	#define scanner context->scanner
%}

%%

start: actions
	  	{ context->result += $1; if (context->wantabort()) YYABORT; }
	;

actions:  actions action
	  	{
			context->result+=$2;
			if ( $2 != 0 ) {
				cout << "failed ("<<$2<<") after executing " << endl;
				context->showPosition(5);
				YYABORT;
			}
			if (context->wantabort()) {
				YYABORT; 
			}
			if ( context->isInteractive() ) { cout << context->Name() << " > " << flush; }
			$$ = $1 + $2;
		}
	//	| actions error ';'
	// do not recover from errors! calling context has to decide what to do and will eventually restart this parser
	//  	{
	//		YYABORT;
	//		//if ( context->isInteractive()) cout << context->Name() << " > " << flush;
	//	}
	| actions T_EXIT
	  	{ context->result += $1; YYACCEPT; }
	| /* empty */
		{ $$ = 0; }
	;

action:
	ESCAPE
		{
			// execute escape in a sub-context. it has its own scope!
			$$ = context->run_subcontext($1, "escape");
		}
	| ';'	{ /*ignore empty statement*/ $$ = 0; }
	| T_LOAD filename ';'
		{
			{
				if (context->isInteractive()) cerr << "loading " << $2 << endl;
				context->loadFile($2);
				if (context->isInteractive()) cerr << "load of " << $2 << " complete" << endl;
				$$ = 0;
			}
		}
	| T_ISSET NAME ';'
		{
			if ( context->var_isset($2) ) {
				cerr << "true" << endl;
			} else {
				cerr << "false" << endl;
			}
			$$ = 0;
		}
	| T_ECHO EXPRESSION ';'
		{
			try {
				cout << $2->String() << endl;
			}
			catch (errno_exception<ENOMSG> &e) {
				cout << "UNDEFINED: " << e.what() << endl;
				//YYABORT;
			}
			$$ = 0;
		}
	| T_SLEEP INTEGER ';'
		{
			sleep($2);
			$$ = 0;
		}
	| T_SUBSCRIBE NAME T_ON ABONAME ';'
		{
			try {
				context->subscribe($2, $4);
				$$ = 0;
			}
			catch (general_exception &e) {
				cout << "subscription failed: " << e.what() << endl;
				$$ = 1;
			}
		}
	| T_UNSUBSCRIBE ABONAME ';'
		{
			try {
				context->unsubscribe($2);
			}
			catch (errno_exception<EFAULT>& e) {
				YYABORT;
			}
			$$ = 0;
		}
	| T_SEND ABONAME EXPRESSION ';'
		{
			string name($2);
			string data = $3->String();
			if (data[data.size()-1] != '\n') data += "\n";
			context->send(name,data);
			$$ = 0;
		}
	| T_SEND ABONAME PACKET_TYPE EXPRESSION ';'
		{
			//packet_t packet;
			string name($2);
			string data = $4->String();	
			if (data[data.size()-1] != '\n') data += "\n";
			//packet.setName($2);
			//packet.type = $3;
			//packet.pprintf("%s", data.c_str());
			//if (context->scptr != NULL) { context->scptr->sendpacket(packet); } else { cout << "scptr==NULL" << endl; }
			context->send(name, data, (packet_type_t)$3);
			$$ = 0;
		}
	| T_SETID NAME ';'
		{
			if (context->scptr != NULL) { context->scptr->setid($2); } else { cout << "scptr==NULL" << endl; }
			$$ = 0;
		}
	| T_WAITFOR ABONAME ':' NAME ';'
		{
			//if (context->isInteractive()) cout << "waiting for " << $2 << ":" << $4 << "..." << endl;
			try {
				context->waitfor($2, $4);
				$$ = 0;
			}
			catch (errno_exception<ENOPROTOOPT> &e) {
				cerr << "could not waitfor " << $2 << ":" << $4 << ": " << e.what() << endl;
				$$ = 1;
			}
		}
	| T_PCALL NAME ';'
		{
			try {
				context->pcall($2);
				$$ = 0;
			}
			catch (errno_exception<EINVAL> &e) {
				cerr << "invalid call of proc \"" << $2 << "\": " << e.what() << endl;
				$$ = 1;
			}
		}
	| NAME '=' ESCAPE ';'
		{
			context->var_set($1, $3);
			$$ = 0;
		}
	| T_LIST T_ABOS ';'	// list active context subscribed abos
		{
			cout << "--- Subscribed abos ---" << endl;
			context->abo_list();
			$$ = 0;
		}
	| T_LIST T_TYPES ';'	// list active contexts packet types
		{
			cout << "--- Packet types ---" << endl;
			context->pkt_list();
			$$ = 0;
		}
	| T_LIST T_VARS ';'	// list active contexts packet types
		{
			cout << "--- Variables ---" << endl;
			context->var_list();
			$$ = 0;
		}
	| T_LIST T_PROCS ';'	// list active contexts packet types
		{
			cout << "--- Procedures ---" << endl;
			context->proc_list();
			$$ = 0;
		}
	| T_LIST ';'	// list active contexts contents
		{
			//cout << "--- Variables ---" << endl;
			//context->var_list();
			cout << "---" << context->Name() << " Packet types ---" << endl;
			context->pkt_list();
			cout << "---" << context->Name() << " Subscribed abos ---" << endl;
			context->abo_list();
			cout << "---" << context->Name() << " Procedures ---" << endl;
			context->proc_list();
			cout << "---" << context->Name() << " Context Tree ---" << endl;
			context->showTree("\t");
			cout << "---" << context->Name() << " Threads ---" << endl;
			context->Thread()->showTree("\t");

			$$ = 0;
		}
	| T_SHOW T_PROCEDURE NAME ';'
		{
			context->proc_show($3); // all
			$$ = 0;
		}
	| T_SHOW T_PROCEDURES ';'
		{
			context->proc_show(); // all
			$$ = 0;
		}
	| T_SHOW EXPRESSION ';'
		{	
			try {
				cout << "Integer_t: " << $2->Integer() << endl;
				cout << "Double_t:  " << $2->Double() << endl;
				cout << "String_t:  \"" << $2->String() << "\"" << endl;
				cout << "Bool_t:    " << $2->Bool() << endl;
			}
			catch (errno_exception<ENOMSG> &e) {
				cout << "UNDEFINED: " << e.what() << endl;
				//YYABORT;
			}
			$$ = 0;
		}
	| T_SHOW T_POS ';'
		{
			context->showPosition();
			$$ = 0;
		}
	| T_SHOW T_TREE ';'
		{
			context->showTree();
			$$ = 0;
		}
	| T_SHOW T_NAME ';'
		{
			cout << "context name " << context->Name() << endl;
			$$ = 0;
		}
	| NAME ';'	// evaluate in subcontext
		{
			$$ = context->proc_run($1);
			$$ = 0;
		}
	| T_WHILE '(' EXPRESSION ')' ESCAPE ';'
		{
			try {
				int ret=0;
				while ( $3->Bool() && ret==0 && !context->wantabort() ) {	// FIXME: this kind of abort() is not the right way! if subcontext hangs, this will not work...
					if (context->isInteractive()) cerr << "WHILE true, action... {" << $5 << "}" << endl;
					ret = context->run_subcontext($5, "while");
					cout << "while subcontext = " << ret << endl;
				}
				$$ = context->wantabort();
			}
			catch (errno_exception<EINVAL> &e) { // value unknown
				if (context->isInteractive()) cerr << "WHILE catched EINVAL exception while parsing \"" << $5 << "\": " << e.what() << endl;
				$$ = -1;
				//YYERROR;
			}
			catch (errno_exception<ENOMSG> &e) { // value unknown
				if (context->isInteractive()) cerr << "WHILE does not know the value of \"" << $3->Name() << "\": " << e.what() << endl;
				$$ = -1;
				//YYABORT;
			}
		}
	| T_BREAK ';'
		{
			$$ = -1;
		}
	| T_IF '(' EXPRESSION ')' ESCAPE ';'
		{
			try {
				if ( $3->Bool() ) {
					if (context->isInteractive()) cerr << "IF true, action..." << endl;
					$$ = context->run_subcontext($5, "if_then");
				} else {
					if (context->isInteractive()) cerr << "IF false, no action." << endl;
					$$ = 0;
				}
			}
			catch (errno_exception<ENOMSG> &e) { // value unknown
				if (context->isInteractive()) cerr << "IF does not know the value of \"" << $3->Name() << "\": " << e.what() << endl;
				$$ = 0;
			}
		}

	| T_IF '(' EXPRESSION ')' ESCAPE T_ELSE ESCAPE ';'
		{
			try {
				if ( $3->Bool() ) {
					if (context->isInteractive()) cerr << "IF true, action..." << endl;
					$$ = context->run_subcontext($5,"if_then");	// load "then" case
				} else {
					if (context->isInteractive()) cerr << "IF false, action..." << endl;
					$$ = context->run_subcontext($7,"if_else");	// load "else" case
				}
			}
			catch (errno_exception<ENOMSG> &e) { // value unknown
				if (context->isInteractive()) cerr << "IF does not know the value of \"" << $3->Name() << "\": " << e.what() << endl;
				//YYABORT;
				$$ = 0;
			}
		}
	| T_NEW T_PACKET_TYPE NAME '(' SENSLIST ')' ';'
		{
			sens_list_t* sl = $5;
			packet_description_t pd($3);
			for (sens_list_t::iterator i=sl->begin(); i!=sl->end(); ++i) {
				pd.push_back(*i);
			}
			sl->clear();	// free memory (all data has been copied into pd)
			try {
				context->pkt_add(pd);
				$$ = 0;
			}
			catch (errno_exception<EEXIST> &e) {
				if (context->isInteractive()) cout << "could not define packet type: " << e.what() << endl;
				$$ = 1;
			}
		}
	| T_NEW T_PROCEDURE NAME ESCAPE ';'
		{
			context->proc_add($3, $4);
			$$ = 0;
		}
	;

filename:	QUOTED;

SENSLIST:	SENSLIST SENSVALUE
	{
		$1+=0;	// just to suppress "warning: unused value: $1" message
		context->senslist.push_back($2);
		$$ = &(context->senslist);
	}
	|
	/* empty */	{ $$ = 0; }
	;

SENSVALUE: T_REAL NAME
	{
		$$ = new valueT<Double_t>($2, "");	// FIXME: include units here
	}
	|  T_INTEGER NAME
	{
		$$ = new valueT<Integer_t>($2, "");	// FIXME: include units here
	}
	|  T_STRING NAME
	{
		$$ = new valueT<String_t>($2, "");	// FIXME: include units here
	}
	|  T_CONST T_STRING QUOTED
	{
		$$ = new valueconstT<String_t>("unnamed", "", $3, value_t::NO_READ_UNIT);	// FIXME: include units here
	}
	|  T_CONST T_STRING NAME QUOTED
	{
		$$ = new valueconstT<String_t>($3, "", $4, value_t::NO_READ_UNIT);	// FIXME: include units here
	}
	;

EXPRESSION:
	  REAL		{ $$ = new valueconstT<Double_t>("constdouble", "", $1);  }
	| INTEGER	{ $$ = new valueconstT<Integer_t>("constinteger", "", $1); }
	| QUOTED	{ $$ = new valueconstT<String_t>("conststring", "", $1); /*cerr << "new quoted \"" << $1 << "\"=\"" << $$->String() << "\"" << endl;*/  }
	| BOOL		{ $$ = new valueconstT<Bool_t>("constbool", "", $1);    }
	| T_ISDEF '(' NAME ')'	{ $$ = new valueconstT<Bool_t>("constbool", "", context->pkt_isdef($3));    }
	| T_ISNDEF '(' NAME ')'	{ $$ = new valueconstT<Bool_t>("constbool", "", !context->pkt_isdef($3));    }
	| NAME
	{
		try {
			$$ = context->lookup($1);
		}
		catch (errno_exception<ENOMSG> &e) {
			cout << e.what() << endl;
			YYERROR;	// throw yacc syntax error
		}
	}
	| ABONAME ':' NAME ':' NAME
	{
		try {
			$$ = context->fqlookup($1, $3, $5);
		}
		catch (errno_exception<ENOMSG> &e) {
			cout << e.what() << endl;
			YYERROR;	// throw yacc syntax error
		}
	}
	| '(' EXPRESSION ')' { $$ = $2 }
	//| EXPRESSION '+' EXPRESSION		{ $$ = new sens_sum_t($1, $3); }
	| EXPRESSION '.' EXPRESSION		{ $$ = new sens_dot_t($1,$3); }
	| EXPRESSION '-' EXPRESSION		{ $$ = new sens_diff_t($1,$3); }
	| EXPRESSION '+' EXPRESSION		{ $$ = new sens_add_t($1,$3); }
	| EXPRESSION '*' EXPRESSION		{ $$ = new sens_mul_t($1,$3); }
	| EXPRESSION '/' EXPRESSION		{ $$ = new sens_div_t($1,$3); }
	| EXPRESSION '%' EXPRESSION		{ $$ = new sens_mod_t($1,$3); }
	| EXPRESSION '<' EXPRESSION		{ $$ = new sens_lt_t($1,$3); }
	| EXPRESSION '>' EXPRESSION		{ $$ = new sens_gt_t($1,$3); }
	| EXPRESSION T_EQ_EQ EXPRESSION		{ $$ = new sens_eq_t($1,$3); }
	| EXPRESSION T_GREATER_EQ EXPRESSION	{ $$ = new sens_gteq_t($1,$3); }
	| EXPRESSION T_LESS_EQ EXPRESSION	{ $$ = new sens_lteq_t($1,$3); }
	| EXPRESSION T_NOT_EQ EXPRESSION	{ $$ = new sens_neq_t($1,$3); }
	//| EXPRESSION T_PLUS_EQ EXPRESSION	{ $$ = $1->operator+=($3); }
	//| EXPRESSION T_MINUS_EQ EXPRESSION	{ $$ = $1->operator-=($3); }
	//| EXPRESSION T_MUL_EQ EXPRESSION	{ $$ = $1->operator*=($3); }
	//| EXPRESSION T_DIV_EQ EXPRESSION	{ $$ = $1->operator/=($3); }
	//| EXPRESSION T_MOD_EQ EXPRESSION	{ $$ = $1->operator%=($3); }
	//| EXPRESSION T_PLUS_PLUS EXPRESSION	{ $$ = $1->operator++($3); }
	//| EXPRESSION T_MINUS_MINUS EXPRESSION	{ $$ = $1->operator--($3); }
	;
	

%%

