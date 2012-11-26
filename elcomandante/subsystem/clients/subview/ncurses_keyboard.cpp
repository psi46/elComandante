/**
 * \file ncurses_keyboard.cpp
 * \author Dennis Terhorst
 * \date Thu Aug 20 18:28:07 CEST 2009
 */
#include "ncurses_keyboard.h"
#include <unistd.h>

int ncurses_keyboard::getchar() {
	return getch();
}
