/**
 * \file global.h
 * \author Dennis Terhorst
 * \date Wed Sep  9 16:10:16 CEST 2009
 */
#ifndef GLOBAL_H
#define GLOBAL_H
#include "ncurses_screen.h"
#include "selectable_sclient.h"

extern ncurses wout;
typedef struct {
	int wantexit;
	sclient* meptr;
} GLOBAL_VARS_T;

extern GLOBAL_VARS_T global;
#endif // ndef GLOBAL_H
