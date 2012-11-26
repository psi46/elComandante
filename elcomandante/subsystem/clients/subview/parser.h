/**
 * \file parser.h
 * \author Dennis Terhorst
 * \date Thu Aug 20 17:46:48 CEST 2009
 */
#ifndef PARSER_H
#define PARSER_H
#include <iostream>	// cerr
#include <unistd.h>	// pipe, write, close
#include <stdio.h>	// fclose


int yyparse(void);
extern FILE* yyin;

#include <string.h>
#include "error.h"
int parse(const char* line) {
	int fd[2];
	if (pipe(fd) < 0) {
		eperror("Couldn't open pipe");
		return -1;
	}
	if (yyin != NULL) {
		fclose(yyin);
	}
	yyin = fdopen(fd[0],"r");
	if ((unsigned int)write(fd[1], line, strlen(line)) != strlen(line)) {
		eperror("Couldn't write line to parser pipe");
		return -1;
	}
	close(fd[1]);
	return yyparse();
}

#ifdef __cplusplus
#include <string>
int parse(std::string line) {
	int fd[2];
	if (pipe(fd) < 0) {
		std::cerr << "Couldn't open pipe" << std::endl;
		return -1;
	}
	if (yyin != NULL) { //FIXME
		fclose(yyin);
	}
	yyin = fdopen(fd[0],"r");
	if ((unsigned int)write(fd[1], line.c_str(), line.size()) != line.size()) {
		std::cerr << "Couldn't write line to parser pipe" << std::endl;
		return -1;
	}
	close(fd[1]);
	return yyparse();
}
#endif //def __CPLUSPLUS__


#endif //ndef PARSER_H
