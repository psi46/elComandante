/**
 * \file globals.h
 * \author Dennis Terhorst
 * \date Fri Oct 30 18:03:52 CET 2009
 */
#ifndef GLOBALS_H
#define GLOBALS_H

//namespace myglobals {
#include "action_context.h"
#include <subsystem/sclient.h>
#include <iostream>

/** \brief processes global variables */
struct globals {
	std::istream* main_is;
	action_context* context;
	int wantexit;
	sclient* scptr;
	// constructor
	globals();
};
//};

extern globals global;
//using namespace myglobals; // FIXME: is this a good idea here?

#endif //ndef GLOBALS_H
