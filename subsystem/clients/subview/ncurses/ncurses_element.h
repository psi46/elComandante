/**
 * \file ncurses_element.h
 * \author Dennis Terhorst
 * \date Mon Sep  7 00:13:18 CEST 2009
 */
#ifndef NCURSES_ELEMENT_H
#define NCURSES_ELEMENT_H
#include <iostream>

//class ncurses_element;
//#include "ncurses_screen.h"
class ncurses;

class ncurses_element {
private:
	ncurses* myParent;
public:
	bool visible;	// shown on screen
	bool enabled;	// accepts keyboard input

	ncurses_element(ncurses* ParentWindow);
	virtual ~ncurses_element();

	// ncurses_element viruals:
	virtual unsigned int row()=0;
	virtual unsigned int column()=0;
	virtual unsigned int width()=0;
	virtual unsigned int height()=0;
	virtual void redraw()=0;
	virtual ncurses* getScreen();
}; // end class ncurses_element
#endif //ndef NCURSES_ELEMENT_H
