/**
 * \file ncurses_textbox.h
 * \author Dennis Terhorst
 * \date Sun Sep  6 20:37:23 CEST 2009
 */

#include "ncurses_textbox.h"

//namespace ncurses {

textbox::textbox(ncurses* Screen, int Line, int Col, int Maxlen) : ncurses_element(Screen) {
	screen=Screen;
	line=Line;
	col=Col;
	maxlen=Maxlen;
}
//virtual 
textbox::~textbox() {}

#include "global.h"
//virtual
int textbox::input(int keycode) {
	if (31 < keycode && keycode < 127) {
		if ( text.size() < (unsigned int)maxlen ) {
			text.push_back((char)keycode);
		} else {
			screen->Bell();
		}
	} else if (keycode == KEY_BACKSPACE) {	if (text.size()>0) text.erase(text.size()-1);
	} else {
		wout << "unknown key in textbox: "<< keycode << "\n";
		//wout << "keycode " << keycode << " ("<< KEY_BACKSPACE << " is BS)\n";
		screen->Bell();
		return -1;
	}
	return 0;
}

// ncurses_element viruals:
//virtual
void textbox::redraw() {
	std::string fill;
	for (int i=maxlen-text.size(); i>0; --i)
		fill.push_back(' ');
	(*screen)(line, col) << "[" << text << fill << "]";
}
//virtual
void textbox::place_cursor() {
	wout << "tabstop to line" << this->row() << " col" << this->column() << "\n";
	(*screen)(this->row(),this->column()) << "";
}

//virtual
unsigned int textbox::row() { return line; }
//virtual
unsigned int textbox::column() { return col; };
//virtual
unsigned int textbox::width() { return maxlen; };
//virtual
unsigned int textbox::height() { return 1; };

std::string textbox::get_text() { return text; };

void textbox::set_text(std::string Text) { text=Text; }

//}; // end namespace
