/**
 * \file valued_t.cpp
 * double value kind of value_t
 *
 * \author Dennis Terhorst
 * \date So 28. Jun 14:49:35 CEST 2009
 */
#include "valued_t.h"
#include <math.h>
#include <stdlib.h>
#include <sstream>
using namespace std;

valued_t::valued_t(string Unit, flags_t Flags) : value_t(Unit, Flags) {
	value = 0;
}

int valued_t::scan_value(char*& ptr) {
	char* eptr=ptr;
	skip_ws(eptr);
	value = strtod(ptr, &eptr);
	if (eptr == ptr) { return -1; }
	ptr= eptr;
	return 0;
}

String_t valued_t::String() {
	ostringstream os(ostringstream::out);
	os << *this;
	return os.str();
}
Double_t valued_t::Double() {
	return value;
}
Integer_t valued_t::Integer() {
	return (Integer_t)value;
}
//
// GLOBAL SCOPE STREAM OUT OPERATOR
//
ostream& operator<<(ostream& os, valued_t& val) {
	if ( (val.flags&value_t::NO_WRITE_UNIT) ) return os << val.value*pow(10,val.exp);

	if (val.exp==0) return os << val.value << "" << val.unit;
	os << val.value;
	switch (val.exp) {
	case  15:	os << "P"; break;
	case  12:	os << "T"; break;
	case   9:	os << "G"; break;
	case   6:	os << "M"; break;
	case   3:	os << "k"; break;
	case  -3:	os << "m"; break;
	case  -6:	os << "u"; break;
	case  -9:	os << "n"; break;
	case -12:	os << "p"; break;
	case -15:	os << "f"; break;
	case -18:	os << "a"; break;
	default:	os << "*10^" << val.exp;
	}
	return os << val.unit;
}

