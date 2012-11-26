/**
 * \file abo_info.cpp
 * \author Dennis Terhorst
 * \date Sun Sep 13 19:02:42 CEST 2009
 */
#ifndef ABO_INFO_H
#define ABO_INFO_H


#include <string>
#include <deque>
#include <time.h>
#include <subsystem/sclient.h>

#include "ncurses_screen.h"

class abo_info {
	time_t lastrx;
	typedef std::deque<packet_t> packetlist_t;
	packetlist_t packets;
public:
	abo_info();
	virtual ~abo_info() {};

	void received(packet_t* packet);
	time_t lastRX();
	void setStorelines(unsigned int i);
	int getStorelines();
	void redraw_to(ncurses& win);
//	friend ncurses& operator<<(ncurses& nc, abo_info& ai);
};

//ncurses& operator<<(ncurses& nc, abo_info& ai);

#endif //ndef ABO_INFO_H
