/**
 * \file selectable_ncurses_keyboard.h
 * \author Dennis Terhorst
 * \date Thu Aug 20 18:18:27 CEST 2009
 */
#ifndef SELECTABLE_NCURSES_KEYBOARD_H
#define SELECTABLE_NCURSES_KEYBOARD_H

#include <ncurses.h>
#include "selectable.h"
#include "ncurses_keyboard.h"

class ncurses_keyboard_selectable : public selectable, public ncurses_keyboard {
public:
	ncurses_keyboard_selectable() : selectable(), ncurses_keyboard() {};
	virtual ~ncurses_keyboard_selectable() {};

	// selectable virtuals
	virtual int getfd();
	virtual int getchecks();
};

#endif //ndef SELECTABLE_NCURSES_KEYBOARD_H

