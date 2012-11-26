/**
 * \file ncurses_textbox.h
 * \author Dennis Terhorst
 * \date Sun Sep  6 20:37:23 CEST 2009
 */
#ifndef NCURSES_TEXTBOX_H
#define NCURSES_TEXTBOX_H

#include "ncurses_screen.h"
#include "ncurses_screen_tabstop.h"
#include <string>
//namespace ncurses {

class textbox : public ncurses_element, public ncurses_tabstop {
private:
	ncurses* screen;
	int line;
	int col;
	int maxlen;
	std::string text;
public:
	textbox(ncurses* Screen, int Line, int Col, int maxlen=0);
	virtual ~textbox();

	/// process the given keycode and return 0, or negative if key not handled.
	virtual int input(int keycode);

	// ncurses_element viruals:
	virtual unsigned int row();
	virtual unsigned int column();
	virtual unsigned int width();
	virtual unsigned int height();
	virtual void redraw();

	// ncurses_tabstop viruals:
	virtual void place_cursor();

	/// retrun textbox content
	std::string get_text();

	void set_text(std::string Text);
}; // end class textbox

//}; // end namespace
#endif //ndef NCURSES_TEXTBOX_H
