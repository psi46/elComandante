/*
 *	masterpage.cpp 24-Oct 2007
 *	Jochen Steinmann 	<jochen.steinmann@rwth-aachen.de>
 */

#include "masterpage.h"
#include "aboinfo.h"

void masterpage::draw() {
//	pprintf(2,3, "Number of subscribed abos: %d", abomap.size() );
	pprintf(3,3, "-----------------------------------------");
//	int l=4;
//	for (abomap_t::iterator abo=abomap.begin(); abo!=abomap.end(); ++abo) {
//		pprintf(l,3, "%s", (*abo).first.c_str()); // name
//		for (int history=0; history<abo->second.HistoryLength(); ++history) {
//			packet_t p=abo->second.Packet(history);
//			pprintf(l,3, "%s", p.data()); // contents
//		}
//	}
}




