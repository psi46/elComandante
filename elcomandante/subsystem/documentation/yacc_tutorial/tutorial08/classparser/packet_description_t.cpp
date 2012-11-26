/**
 * \file packet_description_t.cpp
 * describes the different data fields in a packet
 *
 * \author Dennis Terhorst
 * \date Wed Jul  1 17:36:39 CEST 2009
 */

#include "packet_description_t.h"
using namespace std;

const char* packet_type_name[] = {
	"PKT_MANAGEMENT",	//=0,
	"PKT_DATA",		//=1,
	"PKT_SUBSCRIBE",	//=2,
	"PKT_UNSUBSCRIBE",	//=3,
	"PKT_SUPPLY",		//=4,
	"PKT_UNSUPPLY",		//=5,
	"PKT_CLIENTTERM",	//=6,
	"PKT_SERVERTERM",	//=7,
	"PKT_SETDATA"		//=8
};

packet_description_t::packet_description_t(std::string Name, packet_type_t Type) {
	name = Name; type=Type;
	hasdata = false;
};

packet_description_t::packet_description_t(const packet_description_t& copy) {
	name = copy.name; type = copy.type;
	for ( sens_vec_t::const_iterator s = copy.sens.begin(); s!=copy.sens.end(); ++s) {	// copy senses
		sens.push_back((*s)->copy());
	}
	hasdata = false;
}

//virtual 
packet_description_t::~packet_description_t() {
	for (sens_vec_t::iterator s=sens.begin(); s!=sens.end(); ++s) delete (*s);
};


string packet_description_t::Name() const throw() { return name; }
packet_type_t packet_description_t::Type() const throw() { return type; }

void packet_description_t::push_back(sens_value_t* sen) {
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
	
	// scan pass
	for (sens_vec_t::iterator iter = sens.begin(); iter != sens.end(); ++iter) {
		if ( (*iter)->scan(ptr) < 0 ) { return -2; } // discard if parse failed
	}
	if (strlen(ptr) > 0) { return -1; } // discard packet if not parsed completely

	// accept pass
	for (sens_vec_t::iterator iter = sens.begin(); iter != sens.end(); ++iter) {
		(*iter)->accept();
	}
	hasdata = true;
	return strlen(ptr);
}

bool packet_description_t::hasData() const throw() { return hasdata; }

bool packet_description_t::get_sens(std::string name, sens_value_t*& theSens) {
	for (sens_vec_t::iterator iter = sens.begin(); iter != sens.end(); ++iter) {
		if ( (*iter)->Name() == name ) {
			theSens = (*iter);
			return true;
		}
	}
	return false;
}

bool packet_description_t::operator==(const packet_description_t& other) const throw() {
	if (other.type != type) return false;
	//if (other.name != name) return false;
	return true;
}

bool packet_description_t::operator==(const std::string& Name) const throw() {
	if (name == Name) return true;
	return false;
}

sens_value_t* packet_description_t::lookup(std::string name) throw(errno_exception<ENOMSG>) {
	for (sens_vec_t::iterator sen=sens.begin(); sen!=sens.end(); ++sen) {
		if ( (*sen)->Name() == name ) return (*sen);
	}
	throw errno_exception<ENOMSG>("no such sensation in this packet");
}

//
// GLOBAL
//
ostream& operator<<(ostream& os, packet_description_t* pd) {
	os << "(" << packet_type_name[pd->type] << ": ";
	for (packet_description_t::sens_vec_t::iterator iter=pd->sens.begin(); iter != pd->sens.end(); ++iter) {
		if (iter !=pd->sens.begin()) { os << ", "; }
		try {
			os << (*iter)->Name() << "=" << (*iter)->String();
		}
		catch (errno_exception<ENOMSG> &e) {
			os << (*iter)->Name() << " UNDEFINED";
		}
	}
	return os << ")";
}
ostream& operator<<(ostream& os, packet_description_t& pd) {
	os << "(" << packet_type_name[pd.type] << ": ";
	for (packet_description_t::sens_vec_t::iterator iter=pd.sens.begin(); iter != pd.sens.end(); ++iter) {
		if (iter !=pd.sens.begin()) { os << ", "; }
		try {
			os << (*iter)->Name() << "=" << (*iter)->String();
		}
		catch (errno_exception<ENOMSG> &e) {
			os << (*iter)->Name() << " UNDEFINED";
		}
	}
	return os << ")";
}
