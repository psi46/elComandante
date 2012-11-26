/**
 * \file valuesconst_t.cpp
 * string value kind of value_t
 *
 * \author Dennis Terhorst
 * \date So 28. Jun 14:54:47 CEST 2009
 */

#include "valuesconst_t.h"
#include <string.h>
#include <sstream>
using namespace std;

valuesconst_t::valuesconst_t(String_t Val) : value_t("", NO_WRITE_UNIT | NO_READ_UNIT) {
	value = Val;
}

int valuesconst_t::scan_value(char*& ptr) {
	char* eptr=ptr;
	skip_ws(eptr);
	int ret;
	if ( (ret=strncmp(eptr, value.c_str(), value.length() )) == 0 ) {
		eptr += value.length();
		ptr=eptr;
		return 0;
	}
	//cout << "did not match \"" << value.c_str() << "\"("<< strlen(value.c_str()) <<") with \"" << eptr << "\" return "<< ret << endl;
	return -1;
}

String_t valuesconst_t::String() {
	ostringstream os(ostringstream::out);
	os << *this;
	return os.str();
}
Double_t valuesconst_t::Double() {
	return 0.0;	// FIXME maybe return strtod() here?!?
}
Integer_t valuesconst_t::Integer() {
	return 0;	// FIXME maybe return strtol(,0) here??!
}


//
// GLOBAL SCOPE STREAM OUT OPERATOR
//
ostream& operator<<(ostream& os, valuesconst_t& val) {
	return os << val.value;
}

