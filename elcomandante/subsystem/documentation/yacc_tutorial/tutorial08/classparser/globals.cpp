/**
 * \file globals.cpp
 * \author Dennis Terhorst
 * \date Fri Oct 30 18:06:30 CET 2009
 */
#include "globals.h"

//namespace myglobals {
//	std::istream* main_is;
//	int wantexit;

globals::globals() {
	scptr = NULL;
	main_is = &std::cin;
	context = NULL;
	wantexit = 0;
}

globals global;
//};
