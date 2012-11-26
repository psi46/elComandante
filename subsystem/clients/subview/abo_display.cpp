/**
 * \file abo_display.cpp
 * \author Dennis Terhorst
 * \date Sun Sep 13 19:02:42 CEST 2009
 */
#include "abo_display.h"

//abo_display::abo_display(ncurses *ParentWindow=NULL, int nlines=0, int ncols=0, int line=-1, int col=-1)
abo_display::abo_display(ncurses *ParentWindow, int nlines, int ncols, int line, int col)
 : ncurses_element(ParentWindow),
   window(ParentWindow, nlines, ncols, line, col) {
	//active_abo=infos.end();
}

void abo_display::display(packet_t& packet) {
	infos[packet.name()].received(&packet);
/*		window << BOLD << packet.name() << NOBOLD << " (";
	window << COLOR(1) << packet_type_name(packet.type);
	window << NOCOLOR(1);
	window << "):" << NOBOLD << "\n";
	window << packet.data();		// FIXME: some fancy display here!
	//wout << "(" << packet.data() << ")\n";*/
}
void abo_display::selectvalid() {
	if (infos.find(active_abo) == infos.end() ) {
		active_abo = infos.begin()->first;
	}
}

//virtual
int abo_display::input(int keycode) {
	return 0;
}

/// ncurses_tabstop virtuals:
///@{
//virtual
void abo_display::place_cursor () {
	window(0,0);
}
///@}

/// ncurses_element virtuals:
///@{
//virtual
void abo_display::redraw() {
	window.clear();
	infomap_t::iterator active = infos.find(active_abo);
	for (infomap_t::iterator info= infos.begin(); info!=infos.end(); ++info) {
		// (*info).first : packet name
		// (*info).second: abo_info
		if (info == active) window << REVERSE;
		window << BOLD << (*info).first << NOBOLD;
		info->second.redraw_to(window);
		if (info == active) window << NOREVERSE;
	}
	window.redraw();
}
//virtual
unsigned int abo_display::row() { return window.row(); }
//virtual
unsigned int abo_display::column() { return window.column(); };
//virtual
unsigned int abo_display::width() { return window.width(); };
//virtual
unsigned int abo_display::height() { return window.height(); };
///@}

std::string abo_display::packet_type_name(unsigned short type) {
	switch (type) {
	case PKT_MANAGEMENT:  return std::string("PKT_MANAGEMENT");
	case PKT_DATA:        return std::string("PKT_DATA");
	case PKT_SUBSCRIBE:   return std::string("PKT_SUBSCRIBE");
	case PKT_UNSUBSCRIBE: return std::string("PKT_UNSUBSCRIBE");
	case PKT_SUPPLY:      return std::string("PKT_SUPPLY");
	case PKT_UNSUPPLY:    return std::string("PKT_UNSUPPLY");
	case PKT_CLIENTTERM:  return std::string("PKT_CLIENTTERM");
	case PKT_SERVERTERM:  return std::string("PKT_SERVERTERM");
	case PKT_SETDATA:     return std::string("PKT_SETDATA");
	}
	return "unknown";
}
