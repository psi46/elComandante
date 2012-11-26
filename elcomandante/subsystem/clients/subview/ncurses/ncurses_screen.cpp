/**
 * \file ncuses_screen.cpp
 * \author Dennis Terhorst
 * \date Wed Aug 19 18:12:15 CEST 2009
 */
#include "ncurses_screen.h"
ncurses screen;

//namespace ncurses {

ncurses::ncurses(ncurses* ParentWindow, int nlines, int ncols, int begin_y, int begin_x) : ncurses_element(ParentWindow)
{
	flag_autoclear=false;
	tabstop = NULL;
	if (ParentWindow == NULL) {
		initscr();			/* Start curses mode 		  */
		raw();				/* Line buffering disabled	*/
		keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
		noecho();			/* Don't echo() while we do getch */

		start_color();			/* Start color 			*/
		init_pair(0, COLOR_WHITE, COLOR_BLACK);
		init_pair(1, COLOR_RED, COLOR_BLACK);
		// FIXME: catch SIGWINCH here to see window resizes.
		win = stdscr;
		top = 0;
		left = 0;
		mywidth = 0;
		myheight = 0;
	} else {
		if (nlines <= 0) { nlines = ParentWindow->height(); }
		if (ncols <= 0) { nlines = ParentWindow->width(); }
		if (begin_x < 0) { begin_x = (ParentWindow->width()-ncols)/2; }		// center in columns
		if (begin_y < 0) { begin_y = (ParentWindow->height()-nlines)/2; }	// center in rows
		win = derwin(ParentWindow->win, nlines, ncols, begin_y, begin_x);
		top = begin_y;
		left = begin_x;
		mywidth = ncols;
		myheight = nlines;
	}
};

//virtual
 ncurses::~ncurses()
{
	if ( win == stdscr ) {
		endwin();			// End curses mode
	} else {
		delwin(win);			// delete subwindow
	}
};


void ncurses::redraw() {
	if (flag_autoclear) werase(win);
	for (elements_vector::iterator elem=elements.begin(); elem!=elements.end(); ++elem) {
		if ( (*elem)->visible ) 
			(*elem)->redraw();
	}
	if (tabstop != NULL) tabstop->place_cursor();
	redrawwin(win);
	wrefresh(win);
}

void ncurses::Bell() {
	printw("\a");
}

ncurses& ncurses::operator()(int row, int col) { wmove(win, row, col); return *this; }
ncurses& ncurses::operator<<(char* text) { wprintw(win, "%s",text); return *this; }
ncurses& ncurses::operator<<(const char* text) { wprintw(win, "%s",text); return *this; }
ncurses& ncurses::operator<<(std::string text) { wprintw(win, "%s",text.c_str()); return *this; }
ncurses& ncurses::operator<<(int i) { wprintw(win, "%d",i); return *this; }
ncurses& ncurses::operator<<(ncurses_attribute_t attr) {
	switch (attr) {
	case NORMAL:          wattrset(win, A_NORMAL); break;
	case STANDOUT:        wattron(win, A_STANDOUT); break;
	case NOSTANDOUT:      wattroff(win, A_STANDOUT); break;
	case UNDERLINE:       wattron(win, A_UNDERLINE); break;
	case NOUNDERLINE:     wattroff(win, A_UNDERLINE); break;
	case REVERSE:         wattron(win, A_REVERSE); break;
	case NOREVERSE:       wattroff(win, A_REVERSE); break;
	case BLINK:           wattron(win, A_BLINK); break;
	case NOBLINK:         wattroff(win, A_BLINK); break;
	case DIM:             wattron(win, A_DIM); break;
	case NODIM:           wattroff(win, A_DIM); break;
	case BOLD:            wattron(win, A_BOLD); break;
	case NOBOLD:          wattroff(win, A_BOLD); break;
	case PROTECT:         wattron(win, A_PROTECT); break;
	case NOPROTECT:       wattroff(win, A_PROTECT); break;
	case INVIS:           wattron(win, A_INVIS); break;
	case NOINVIS:         wattroff(win, A_INVIS); break;
	case ALTCHARSET:      wattron(win, A_ALTCHARSET); break;
	case NOALTCHARSET:    wattroff(win, A_ALTCHARSET); break;
	case CHARTEXT:        wattron(win, A_CHARTEXT); break;
	case NOCHARTEXT:      wattroff(win, A_CHARTEXT); break;
	case COLOR_CHANGE:    break; // (handled by function)
	}
	return *this;
}
/*ncurses& ncurses::operator<<(ncurses_attribute& ncattr) {
	ncattr.set(win);
	return *this;
}
*/
//ncurses& ncurses::set(int attr) { attron(attr); return(*this); }

// ncurses_element viruals:
//virtual
 unsigned int ncurses::row() { return top; }

//virtual
 unsigned int ncurses::column() { return left; }

//virtual
unsigned int ncurses::width() {
 	if (win == stdscr) { return COLS; }
 	return mywidth;
}

//virtual
 unsigned int ncurses::height() {
 	if (win == stdscr) { return LINES; }
 	return myheight;
}


void ncurses::scrolling(bool onoff) {
	idlok(win, onoff);
	scrollok(win, onoff);
}

void ncurses::autoclear(bool onoff) {
	flag_autoclear = onoff;
}

void ncurses::clear() {
	werase(win);
}
void ncurses::set_background(char c) {
	wbkgdset(win, c);
}

void ncurses::set_tabstop(ncurses_tabstop* ts) {
	tabstop = ts;
}

// GLOBAL FUNCTIONS (implementation)

ncurses_attribute_t COLOR(int n) {	// NOTE: this only works for curwin
	attron(COLOR_PAIR(n));
	return (COLOR_CHANGE);
}
ncurses_attribute_t NOCOLOR(int n) {
	attroff(COLOR_PAIR(n));
	return (COLOR_CHANGE);
}


//}; // end namespace
