/**
 * \file ncuses_screen.h
 * \author Dennis Terhorst
 * \date Wed Aug 19 18:12:15 CEST 2009
 */
#ifndef NCUSES_SCREEN_H
#define NCUSES_SCREEN_H

class ncurses;
//namespace ncurses {

#include <ncurses.h>
#include <stdarg.h>
#include <vector>
#include "ncurses_element.h"
#include "ncurses_screen_tabstop.h"
#include <iostream>
#include <string>

typedef enum {
    NORMAL,        // Normal display (no highlight)
    STANDOUT,      // Best highlighting mode of the terminal.
    UNDERLINE,     // Underlining
    REVERSE,       // Reverse video
    BLINK,         // Blinking
    DIM,           // Half bright
    BOLD,          // Extra bright or bold
    PROTECT,       // Protected mode
    INVIS,         // Invisible or blank mode
    ALTCHARSET,    // Alternate character set
    CHARTEXT,      // Bit-mask to extract a character
    NOSTANDOUT,      // Best highlighting mode of the terminal.
    NOUNDERLINE,     // Underlining
    NOREVERSE,       // Reverse video
    NOBLINK,         // Blinking
    NODIM,           // Half bright
    NOBOLD,          // Extra bright or bold
    NOPROTECT,       // Protected mode
    NOINVIS,         // Invisible or blank mode
    NOALTCHARSET,    // Alternate character set
    NOCHARTEXT,      // Bit-mask to extract a character
//    COLOR_PAIR(n)   // Color-pair number n 
    COLOR_CHANGE
} ncurses_attribute_t;

typedef std::vector<ncurses_element*> elements_vector;
typedef std::vector<ncurses_tabstop*> tabstop_vector;

class ncurses_attribute;
class ncurses : public ncurses_element {
private:
	WINDOW* win;
	int top, left, mywidth, myheight;
	elements_vector elements;
	
	ncurses_tabstop* tabstop;
	bool flag_autoclear;

public:
	friend class ncurses_element;
	/// construct a new window
	/// if ParentWindow is NULL, the constructor initializes ncurses and uses the stdscr.
	/// Else, the dimensions are used relative to ParentWindow to create a new window.
	/// If nlines or ncols is zero, maximum dimensions are used from ParentWindow.
	/// begin_y and begin_x give top-left coordinates inside ParentWindow, or are calculated
	/// to center the new window if negative.
	ncurses(ncurses* ParentWindow = NULL, int nlines=0, int ncols=0, int begin_y = -1, int begin_x = -1);
	virtual ~ncurses();
	void redraw();
	void Bell();
	ncurses& operator()(int row, int col);
	ncurses& operator<<(char* text);
	ncurses& operator<<(const char* text);
	ncurses& operator<<(std::string text);
	ncurses& operator<<(int i);
	ncurses& operator<<(ncurses_attribute_t attr);
//	ncurses& operator<<(ncurses_attribute& attr);
//	ncurses& set(int attr);

	// ncurses_element viruals:
	virtual unsigned int row();
	virtual unsigned int column();
	virtual unsigned int width();
	virtual unsigned int height();

	// own functionality
	void scrolling(bool onoff=true);
	void autoclear(bool onoff=true);
	void clear();
	void set_background(char c);
	void set_tabstop(ncurses_tabstop* ts);
//	friend class ncurses_attribute;
};

//class ncurses;
extern ncurses screen;	// global ncurses init and destruction


/*
class ncurses_attribute {
public:
	virtual ~ncurses_attribute() {};
	virtual void set(WINDOW* win)=0;
};

class COLOR_CLASS : public ncurses_attribute {
	int c;
public:
	COLOR(int n) : ncurses_attribute() {
		c=n;
	}
	virtual ~COLOR() {};
	virtual void set(WINDOW* win) {
		wattron(win, COLOR_PAIR(c));
	}
};

class NOCOLOR_CLASS : public ncurses_attribute {
	int c;
public:
	NOCOLOR(int n) : ncurses_attribute() {
		c=n;
	}
	virtual ~NOCOLOR() {};
	virtual void set(WINDOW* win) {
		wattroff(win, COLOR_PAIR(c));
	}
};*/

// GLOBAL FUNCTIONS (definition)
ncurses_attribute_t COLOR(int n);;
ncurses_attribute_t NOCOLOR(int n);


/*
class ncurses_attribute {
public:
	virtual ncurses& operator()(ncurses&) = 0;
};

class bold : public ncurses_attribute {
	virtual ncurses& operator()(ncurses& nc) { attron(A_BOLD); return nc; }
};
#define BOLD		bold()

class nobold : public ncurses_attribute {
	virtual ncurses& operator()(ncurses& nc) { attroff(A_BOLD); return nc; }
};
#define NO_BOLD		nobold()
*/

//}; // end namespace
#endif //ndef NCUSES_SCREEN_H
