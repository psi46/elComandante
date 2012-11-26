/**
 * \file abo_info.cpp
 * \author Dennis Terhorst
 * \date Sun Sep 13 20:36:08 CEST 2009
 */
#include "abo_info.h"

abo_info::abo_info() { this->setStorelines(1); lastrx=0; }

void abo_info::received(packet_t* packet) {
	time(&lastrx);
	packets.push_back(*packet);
	packets.pop_front();
}

time_t abo_info::lastRX() { return lastrx; }

void abo_info::setStorelines(unsigned int i) { packets.resize(i); }

int abo_info::getStorelines() { return packets.size(); }

void abo_info::redraw_to(ncurses& win) {
	win << " (T+" << time(NULL)-lastrx << ")\n";
	for (packetlist_t::iterator pkt_iter=packets.begin(); pkt_iter!= packets.end(); ++pkt_iter) {
		win << COLOR(1) << pkt_iter->type << " " << pkt_iter->data();
		win << NOCOLOR(1);
	}
}
/*
ncurses& operator<<(ncurses& nc, abo_info& ai) {
	for (abo_info::lines_t::iterator iter=ai.lines.begin(); iter!=ai.lines.end(); ++iter) {
		nc << ">" << (*iter) << "\n";
	}
	return nc;
}*/
