/**
 * \file abo_display.h
 * \author Dennis Terhorst
 * \date Sun Sep 13 19:02:42 CEST 2009
 */
#ifndef ABO_DISPLAY_H
#define ABO_DISPLAY_H
#include "ncurses_screen.h"
#include "ncurses_screen_tabstop.h"
#include <map>
#include <string>
#include <subsystem/sclient.h>

#include "abo_info.h"

class abo_display : public ncurses_element, public ncurses_tabstop {
	typedef std::map<std::string, abo_info> infomap_t;
	infomap_t infos;
	std::string active_abo;
	ncurses window;
	//int active_abo;
public:
	abo_display(ncurses *ParentWindow=NULL, int nlines=0, int ncols=0, int line=-1, int col=-1);

	void display(packet_t& packet);
	void selectvalid();

	virtual int input(int keycode);

	// ncurses_tabstop virtuals:
	virtual void place_cursor ();

	// ncurses_element virtuals:
	virtual void redraw();
	virtual unsigned int row();
	virtual unsigned int column() ;
	virtual unsigned int width() ;
	virtual unsigned int height() ;
private:
	std::string packet_type_name(unsigned short type);
};

#endif //ndef ABO_DISPLAY_H
