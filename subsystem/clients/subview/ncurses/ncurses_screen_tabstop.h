/**
 * \file ncurses_screen_tabstop.h
 * \author Dennis Terhorst
 * \date Thu Sep 10 17:23:13 CEST 2009
 */
#ifndef NCURSES_SCREEN_TABSTOP_H
#define NCURSES_SCREEN_TABSTOP_H

/// abstract class for ncurses tabstops
class ncurses_tabstop {
public:
	virtual ~ncurses_tabstop() {};
	virtual void place_cursor() = 0;
};

#endif //ndef NCURSES_SCREEN_TABSTOP_H
