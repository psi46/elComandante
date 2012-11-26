/**
 * \file subscript.h
 * \author Dennis Terhorst
 * \date Sun Jul 19 16:20:14 CEST 2009
 */

#ifndef SUBSCRIPT_H
#define SUBSCRIPT_H

#include <time.h>

union YYSTYPE {
	int integer;
	double real;
	char* cptr;
	struct keyval_t {
		char* key;
		char* val;
	} keyval;
	struct pttype_t {
		double lat;
		double lon;
		double ele;
		time_t t;
	} pttype;
};
typedef union YYSTYPE YYSTYPE;
#define YYSTYPE_IS_DECLARED

//extern int yydebug;

//int yyparse();

#endif //ndef SUBSCRIPT_H
