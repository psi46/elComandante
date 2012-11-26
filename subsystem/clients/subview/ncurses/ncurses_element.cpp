/**
 * \file ncurses_element.cpp
 * \author Dennis Terhorst
 * \date Mon Sep  7 17:42:16 CEST 2009
 */
#include "ncurses_element.h"
#include "ncurses_screen.h"

ncurses_element::ncurses_element(ncurses* ParentWindow) {
	visible=true;
	enabled=true;
	// append myself to my screens element list
	if (ParentWindow != NULL) {
		ParentWindow->elements.push_back(this);
	}
	myParent=ParentWindow;
}
//virtual
ncurses_element::~ncurses_element() {
	if (myParent != NULL) {
		//for ((myParent->elements)::iterator elem = myParent->elements.begin();
		for (elements_vector::iterator elem = myParent->elements.begin();
						       elem != myParent->elements.end();
							++elem        ) {			// look into my screen's elements
			if ( (*elem) == this ) { myParent->elements.erase(elem); break; }		// and remove myself
		}
	}
}

ncurses* ncurses_element::getScreen() {
	return myParent;
}
