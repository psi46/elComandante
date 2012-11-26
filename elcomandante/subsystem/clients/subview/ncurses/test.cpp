/**
 * \file test.cpp
 * \author Dennis Terhorst
 * \date Wed Aug 19 18:15:03 CEST 2009
 */


/*

class screen : public ncurses_element {
	void redraw() {
		this->redraw_children();
		ncurses::Refresh();
	}
};
*/

/*
 *
#include "ncurses_screen.h"
#include <string.h>
class textbox : public ncurses_element {
	unsigned int myrow, mycol, myheight, mywidth;
	char* text;
public:
	textbox(ncurses_element* Parent, unsigned int Row, unsigned int Col,
	        const char* Text,
	        unsigned int Height=0, unsigned int Width=0) : ncurses_element(Parent) {
		myrow=Row;
		mycol=Col;
		myheight=Height;
		mywidth=Width;
		text = strdup(Text);
	}
	virtual ~textbox() {
		free(text);
	}
	// ncurses_element viruals:
	virtual unsigned int row() { return myrow; }
	virtual unsigned int column() { return mycol; }
	virtual unsigned int width() { return mywidth; }
	virtual unsigned int height() { return myheight; }

	virtual void redraw_children() {
		cerr << "textbox::redraw_children()" << endl;
		cerr << "printing " << text << endl;
		getScreen()(myrow, mycol) << text;
	}
}; // end class textbox
*/

#include "ncurses_screen.h"
int main()
{	
	screen(4,3) << 41 << BOLD;
	screen << 42 << COLOR(1) << NOBOLD << 43;	/* Print Hello World		  */
	screen(5,3) << NOCOLOR(1) << "Hallo Welt";
	screen.redraw();
	getch();

	return 0;
}
