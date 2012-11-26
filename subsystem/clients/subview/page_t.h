/*
 *	page_t.h	12-Oct-2007
 *	Jochen Steinmann	<jochen.steinmann@rwth-aachen.de>
 *	(Documentation: Dennis Terhorst)
 */
#ifndef PAGE_T_H
#define PAGE_T_H


//#define MAX_PAGES	42
#define MAX_TITLELEN	80
#define BUFFERSIZE	500

//#define COLOR

/**
 * \brief ncurses based page class
 *
 * This class provides basic functions of ncurses and helps to organize output
 * to pages. A page is basically one screen full of information. Each page_t
 * derived class overwrites the page_t::draw() function and thus can be redrawn
 * when needed with the redraw() function.
 *
 * The common page_t parent class keeps 
 * track of all instanciated objects in a double linked list (ordered as a ring)
 * and functions like pageup() and pagedown() switch the active page.
 *
 * Some of the mostly used ncurses functions are wrapped in this class and
 * simple derived pages will probably not need to #inlcude <ncurses.h> directly.
 * This eases development of extensions for users not familliar with ncurses.
 */

class page_t {
private:
	static int	nopages;		///< number of pages
	static page_t*	activepage;		///< active page
	page_t* 	next;			///< next page (linked list pointer)
	page_t* 	previous;		///< previous page pointer (linked list pointer)
	static page_t* 	end;			///< end of the page list
	static page_t*	beginning;		///< beginning of the page list
	char 		title[MAX_TITLELEN];	///< title of the page

public:
	// constructor/destructor
	page_t();	
	virtual ~page_t();

	// init
	static void init(); // Function to init ncurses
	static void uninit(); // Function to uninit ncurses

	// draw functions
	static void redraw(); 	// um die aktuelle seite komplett (mit titel, rahmen, etc) neu aufzubauen
	static void drawmenu(); // menu zeichen
	virtual void draw() {};	// fuer die child classes zum in-der-seite-schreiben

	// page functions	
	int setTitle(char* );
	const char* getTitle();	
	int pageWidth();
	int pageHeight();

	static void pageup();
	static void pagedown();
	static void setActivepage(page_t* newact);
	static page_t* getActivepage();
	page_t* getNext();	// probably better private ? (FIXME check this)
	page_t* getPrev();

	// input stub
	virtual void input(char* buffer, int buflen) { return; }

	// print and scan functions
	static int pprintf(int row, int col, char* fmt, ...); // see man va_start
	void pscanstr(char* text, char* buffer);
	void pgetstr(char*, char*);
	int locate(int row, int col);

	static void cursor_on();
	static void cursor_off();
	static void cursor_onner();
	static void setReversed();
	static void setNormal();
	
	static void bouncer();
};

#endif
