/*
 *	page_t.cpp 12-Oct 2007
 *	Jochen Steinmann 	<jochen.steinmann@rwth-aachen.de>
 * 	compile with
 *		g++ -Wall page_t.h page_t.cpp -lncurses -o page
 */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "page_t.h"
#include "input.h"


int page_t::nopages = 0;
page_t* page_t::end = NULL;
page_t* page_t::beginning = NULL;
page_t* page_t::activepage = NULL;



/**
 * Constructor
 *
 * The constructor inserts the new page into the linked list and
 * sets a default title.
 */
page_t::page_t() {
	next = this;
	previous = end;
	if(previous != NULL)
		previous->next = this;
	else
		previous = this;

	if(beginning == NULL)
		beginning = this;

	if (activepage == NULL) {
		activepage = this;	// FIXME: this should not be neccessary, but produces segfaults, when removed
	}
	end = this;
	sprintf(title,"Page %d",nopages);

	nopages++;
}


/**
 * Destructor
 *
 * The destructor removes the node from the linked list.
 */
page_t::~page_t() {
	if(this == beginning) {
		beginning = next;
	}
	if(previous!=NULL) {
		previous->next = next;
	}
	if(next!=NULL) {
		next->previous = previous;
	} else {
		end = previous;
	}
	nopages--;
}


/**
 * Init the nacurses system
 *
 * This function is defined as static and has to be called exactly once
 * before any page_t (or derived class) is instanciated. All necessary
 * initialisation and definition functions of ncurses are called here.
 */
void page_t::init() {

	initscr(); cbreak(); noecho();

	atexit(page_t::uninit);
	clear();		// clear the screen
	
	nonl();			// fowrward nl from curses to user programm
	keypad(stdscr, TRUE);	// let ncruses handle the function key escapes

#ifdef COLOR
	start_color();
	init_pair(2, COLOR_GREEN, COLOR_WHITE); 
	init_pair(1, COLOR_BLACK, COLOR_WHITE); 
        assume_default_colors(COLOR_BLACK, COLOR_WHITE);
	//attrset(A_BOLD | COLOR_PAIR(2));
	color_set(1, 0);
#endif
	refresh();
}


/**
 * Uninit the ncurses system
 *
 * This function does not have to be called directly. It is registered for a
 * call at program termination with \c atexit() command, from page_t::init().
 */
void page_t::uninit() {
	clear();
	page_t::cursor_on();
	echo();
	nl();
	endwin();
}


/**
 * Redraw the page
 *
 * \c clear() the screen and call the activepages \c draw() function and 
 * page_t::drawmenu().
 */
void page_t::redraw() {
//	clear();	// moved to page up/down
//	mvprintw(1,2,"                                       ");
	mvprintw(1,2,"%s",activepage->title);
					//FIXME something more
	activepage->draw();
	page_t::drawmenu();
	wrefresh(stdscr);
}


/**
 * Draw the menu
 */
void page_t::drawmenu() {
	activepage->locate(0,0);
	for (page_t* pg = beginning; pg != NULL ; pg = pg->next) {
		if ( pg == activepage ) {
			printw(" >%s< ", pg->getTitle());
		} else {
			printw("  %s  ", pg->getTitle());
		}
		if ( pg->next == pg ) break;
	};
        int x, y;
        getmaxyx(stdscr,y,x);
        mvprintw(y-2,2,"pages: %d , exit q | PGUP | PGDOWN", nopages);
}



/**
 * Switch to activepage->next
 */
void page_t::pagedown() {
	clear();
	if(activepage == end)
		activepage = beginning;
	else
		activepage = activepage->next;
	page_t::redraw();
}


/**
 * Switch to activepage->previous
 */
void page_t::pageup() {
	clear();
	if(activepage == beginning)
		activepage = end;
	else
		activepage = activepage->previous;
	page_t::redraw();
}


/**
 * Set activepage
 */
void page_t::setActivepage(page_t* newact) {
	if (NULL == newact) return;
	clear();
	activepage = newact;
	page_t::redraw();
}

/**
 * Set activepage
 */
page_t* page_t::getActivepage() {
	return activepage;
} 

/**
 * Return a page_t pointer to the next page
 * 
 * \deprecated The page_t::getNext() function should not be used, as
 * pageup() and pagedown() provide correct handling of the activepage.
 */
page_t* page_t::getNext() {
	return this->next;
}


/**
 * Return a page_t pointer to the previous page
 * 
 * \deprecated The page_t::getPrev() function should not be used, as
 * pageup() and pagedown() provide correct handling of the activepage.
 */
page_t* page_t::getPrev() {
	return this->previous;
}



/**
 * Returns the Title 
 */
const char* page_t::getTitle() {
	return title;
}


/**
 * Set the Title of this page
 */
int page_t::setTitle(char* title) {
	strncpy(this->title,title,MAX_TITLELEN-2);
	this->title[MAX_TITLELEN-1] = '\0';
	return 0;
}


/**
 * Returns the Width of the Screen
 */
int page_t::pageWidth() {
	int x,y;
	getmaxyx(stdscr,y,x);
	return x;
}


/**
 * Returns the Height of the Screen
 */
int page_t::pageHeight() {
	int x,y;
	getmaxyx(stdscr,y,x);
	return y;
}


/**
 * \defgroup grp_cursor Cursor Visibility and Movement Functions
 */
//@{
/**
 * Turn display of the cursor off
 */
void page_t::cursor_off() {
	if ( curs_set(0) == ERR ) fprintf(stderr, "page_t: cursor visibility change not supported\n");
}


/**
 * Display a normal cursor
 */
void page_t::cursor_on() {
	curs_set(1);
}


/**
 * Display the cursor in a special way
 */
void page_t::cursor_onner() {
	curs_set(2);
}


/**
 * Move the cursor on the screen
 */
int page_t::locate(int row, int col) {
	move(row,col);
	return 0;
}
//@}


/**
 * Print on the Screen 
 *
 * This function wraps the ncurses \c mvprintw() function. It is constructed with a
 * variable argument list and behaves like a normal printf but with \p row  and \p col
 * parameters.
 *
 * \param row row to start
 * \param col column to start
 * \param fmt ... Printf() parameters
 * \return return value of mvprintw()
 */
int page_t::pprintf(int row, int col, char* fmt, ...) { // see man va_start 
	va_list argptr;
	char buffer[BUFFERSIZE];
	va_start(argptr, fmt);
	vsnprintf(buffer, BUFFERSIZE-1, fmt, argptr);
	return mvprintw(row, col, buffer);
}


/**
 * Read in a string
 *
 * This function uses the \c getstr() function to read from stdin.
 *
 * \deprecated Use class \ref input instead, because \c getstr() is not
 * compatible with \c select() based i/o multiplexing (see \ref selectable).
 */
void page_t::pgetstr(char* text, char* buffer) {
        int x,y;
        getmaxyx(stdscr,y,x);
        mvprintw(y-3,2,"%s",text);

	getstr(buffer);
}


void page_t::setReversed() {
	attrset(A_REVERSE);
}

void page_t::setNormal() {
	attrset(A_NORMAL);
}

void page_t::bouncer() { 
	static unsigned int s=0;
	//static char dot[] = "_.,·°'`'°·,.";
	static char dot[] = "/-\\|";

	mvprintw(0,0,"%c", dot[s++]);
	refresh();
	if (s>=strlen(dot)) s=0;
	return;
}


//#define DEBUG_WITH_MAIN

#ifdef DEBUG_WITH_MAIN

#include "nodepage_flow.h"
#include "errorpage.h"
#include <stdio.h>

// set this to errorpage::perror()
//#define eprintf(format, args...) fprintf (stderr, format , ##args)
#define eprintf(format, args...) errorpage::eerror(format , ##args)

int main() {
	page_t::init();	
	input inp;

	page_t* test1 = new page_t;
        page_t* test2 = new page_t;
        test2->setTitle("HALLO WELT 2");
//        page_t* test3 = new page_t;
//        nodepage_flow* test4 = new nodepage_flow;
	errorpage* test5 = new errorpage;
	
//        int n=0;

	char buffer[80];
	int buflen=80;

//	for(int i=0; i< 100; i++){
//		errorpage::eerror("Fehler %d", i);
//	}

	eprintf("init complete");

	int testcnt = 0;
        while (1) {
                selectable::run(3,0);

//		inp.setmode(INP_LINE);

                if ( inp.isready(CHK_READ) )    {
			buflen = 80;
                        int ret = inp.readin(buffer, &buflen);
                        // parse command line commands
			
			if(strncmp(buffer,"q",1)==0)	{ break; }
			else
			if(strncmp(buffer, "n",1)==0)	{ page_t::pageup(); }
			else
			if(strncmp(buffer, "p",1)==0)	{ page_t::pagedown(); }
			else
			if(strncmp(buffer, "e",1)==0)	{ eprintf("neuer Fehler %d", testcnt++); }
			else
			eprintf("taste nicht geparsed char[] = 0x%02X 0x%02X 0x%02X 0x%02X ", buffer[0], buffer[1], buffer[2], buffer[3]);
                }

        } // end while(1)

        eprintf("main: normal exit.\n");
	return 0;
}
#endif
