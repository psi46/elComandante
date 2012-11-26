/**
 * \file ncurses_label.h
 * \author Dennis Terhorst
 * \date Sun Sep  6 20:37:23 CEST 2009
 */

#include "ncurses_label.h"

//namespace ncurses {

label::label(ncurses* Screen, int Line, int Col, const char* Text) : ncurses_element(Screen) {
	screen=Screen;
	line=Line;
	col=Col;
	text = strdup(Text);
}
//virtual 
label::~label() {
	free(text);
}

// ncurses_element viruals:
//virtual
void label::redraw() {
	(*screen)(line, col) << text;
}
//virtual
unsigned int label::row() { return line; }
//virtual
unsigned int label::column() { return col; };
//virtual
unsigned int label::width() { return strlen(text); };
//virtual
unsigned int label::height() { return 1; };

void label::set_text(const char* Text) {
	if (Text==NULL) return;
	free(text);
	text=strdup(Text);
}

//}; // end namespace
