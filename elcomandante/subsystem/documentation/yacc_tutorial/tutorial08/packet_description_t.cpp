/**
 * \file packet_description_t.cpp
 * describes the different data fields in a packet
 *
 * \author Dennis Terhorst
 * \date Wed Jul  1 17:36:39 CEST 2009
 */

#include "packet_description_t.h"

packet_description_t::packet_description_t(string AboName, packet_type_t Type) {
	name = AboName; type=Type;
};

//virtual 
packet_description_t::~packet_description_t() {};

void packet_description_t::push_back(sens_value_t* sen) {
//	string sensname = name +":" +sname;
//	cout << "new sens: \"" << sensname << "\"" << endl;
//	sensations[sensname] = sen;
//	cout << "add sens: \"" << sen->Name() << "\"" << endl;
	sens.push_back(sen);
	return;
}

/**
 * reads in all senses from the packet text
 * returns number of left over chars after all senses have been scanned on success
 *  or negative if a sense failed to scan
 */
int packet_description_t::read_packet(const char* text) {
	char* ptr = (char*)text;
	int ret;
	// FIXME: sensations MUST take there values only, if full packet was parsed!
	for (sens_vec_t::iterator iter = sens.begin(); iter != sens.end(); ++iter) {
		//cout << "reading " << (*iter)->Name() << flush;
		ret = (*iter)->scan(ptr);
		if ( ret < 0 ) { return -1; }
		//	cout << "\tcould not read " << *iter << " = " << (*iter)->String() << " (return: " << ret << ")"<< endl;
		//	cout << "PACKET: \""<< ptr << "\"" << endl;
		//	return -1;
		//}
		//cout << " = " << (*iter)->String() << endl;
	}
	return strlen(ptr);
}

bool packet_description_t::get_sens(string name, sens_value_t*& theSens) {
	for (sens_vec_t::iterator iter = sens.begin(); iter != sens.end(); ++iter) {
		if ( (*iter)->Name() == name ) {
			theSens = (*iter);
			return true;
		}
	}
	return false;
}

bool packet_description_t::operator==(packet_description_t& other) {
	if (other.type != type) return false;
	if (other.name != name) return false;
	return true;
}

//
// GLOBAL
//
ostream& operator<<(ostream& os, packet_description_t* pd) {
	os << pd->name << "[" << pd->type << "] (";
	for (packet_description_t::sens_vec_t::iterator iter=pd->sens.begin(); iter != pd->sens.end(); ++iter) {
		if (iter !=pd->sens.begin()) { os << ", "; }
		os << (*iter)->Name();
	}
	return os << ")";
}
