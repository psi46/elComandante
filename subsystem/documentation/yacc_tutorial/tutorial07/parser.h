#ifndef PARSER_H
#define PARSER_H

#include <time.h>

union YYSTYPE {
	int integer;
	double real;
	char* string;
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

int yyparse();

#endif //ndef PARSER_H
