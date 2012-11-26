/**
 * \file ncurses_keyboard.h
 * \author Dennis Terhorst
 * \date Thu Aug 20 18:18:27 CEST 2009
 */
#ifndef NCURSES_KEYBOARD_H
#define NCURSES_KEYBOARD_H

#include <ncurses.h>

class ncurses_keyboard {
public:
	ncurses_keyboard() {};
	virtual ~ncurses_keyboard() {};

	int getchar();
};

#endif //ndef NCURSES_KEYBOARD_H
