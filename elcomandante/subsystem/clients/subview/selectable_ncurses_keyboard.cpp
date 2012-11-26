/**
 * \file selectable_ncurses_keyboard.cpp
 * \author Dennis Terhorst
 * \date Thu Aug 20 18:18:27 CEST 2009
 */
#include "selectable_ncurses_keyboard.h"
#include <unistd.h>
//
// selectable virtuals
//
int ncurses_keyboard_selectable::getfd() {
	return STDIN_FILENO;
}
int ncurses_keyboard_selectable::getchecks() {
	return CHK_READ;
}


