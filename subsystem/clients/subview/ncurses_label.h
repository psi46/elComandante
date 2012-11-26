/**
 * \file ncurses_label.h
 * \author Dennis Terhorst
 * \date Sun Sep  6 20:37:23 CEST 2009
 */
#ifndef NCURSES_LABEL_H
#define NCURSES_LABEL_H

#include "ncurses_screen.h"
#include <string.h>
//namespace ncurses {

class label : public ncurses_element {
private:
	ncurses* screen;
	int line;
	int col;
	char* text;
public:
	label(ncurses* Screen, int Line, int Col, const char* Text);
	virtual ~label();

	// ncurses_element viruals:
	virtual unsigned int row();
	virtual unsigned int column();
	virtual unsigned int width();
	virtual unsigned int height();
	virtual void redraw();

	void set_text(const char* Text);
}; // end class label

//}; // end namespace
#endif //ndef NCURSES_LABEL_H
