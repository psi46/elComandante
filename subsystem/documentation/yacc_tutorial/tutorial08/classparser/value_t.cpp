/**
 * \file value_t.cpp
 * \author Dennis Terhorst
 * \date Tue Oct  6 18:41:26 CEST 2009
 */
#include "value_t.h"
using std::string;

value_t::value_t(std::string name, std::string Unit, flags_t Flags) throw() : sens_value_t(name) {
	unit = Unit;
	exp = 0;
	flags = Flags;
	initialized = false;
}

/*value_t::value_t(string name, const value_t& v) throw() : sens_value_t(name) {
	unit = v.unit;
	exp = 0;
	flags = v.flags;
	initialized = false;
}*/

value_t::~value_t() throw() {};	// destructor must always be virtual!

bool value_t::hasValue() const throw() { return initialized; }

int value_t::scan(char*& ptr) throw() {
	char* oldptr=ptr;
	int ret;
	if ( (ret=this->scan_value(ptr)) < 0 ) { ptr=oldptr; return -1; }
	//cout << "value_t::scan() flags for this value: " << (flags & NO_READ_UNIT?"NO_READ_UNIT ":"") << (NO_WRITE_UNIT&flags?"NO_WRITE_UNIT":"")  << endl; 
	if ( !(flags&NO_READ_UNIT) )
		if ( (ret=this->scan_unit(ptr)) < 0) { ptr=oldptr; return -2; }
	return 0;
} // end scan

void value_t::accept() throw() {	// accept nothing per default (e.g. in value*const classes)
	initialized = true;
}


void value_t::skip_ws(char*& ptr) const throw() {
	while (*ptr == ' ' || *ptr == '\t' ) { ptr++; }
} // end skip_ws


int value_t::scan_unit(char*& ptr) throw() {
	char* eptr=ptr;
	skip_ws(eptr);
	
	// read unit modifier
	     if ( *eptr == 'P' ) { exp = 15; eptr++; /* PETA */ }
	else if ( *eptr == 'T' ) { exp = 12; eptr++; /* TERA */ }
	else if ( *eptr == 'G' ) { exp =  9; eptr++; /* GIGA */ }
	else if ( *eptr == 'M' ) { exp =  6; eptr++; /* MEGA */ }
	else if ( *eptr == 'k' ) { exp =  3; eptr++; /* KILO */ }
	else if ( *eptr == 'm' ) { exp = -3; eptr++; /* MILI */ }
	else if ( *eptr == 'u' ) { exp = -6; eptr++; /* MICR */ }
	else if ( *eptr == 'n' ) { exp = -9; eptr++; /* NANO */ }
	else if ( *eptr == 'p' ) { exp =-12; eptr++; /* PICO */ }
	else if ( *eptr == 'f' ) { exp =-15; eptr++; /* FMTO */ }
	else if ( *eptr == 'a' ) { exp =-18; eptr++; /* ATTO */ }


	if ( strncmp(eptr, unit.c_str(), unit.length() ) == 0 ) {	// unit after mod read
		eptr += unit.length();
		//cout << "scanned unit 1e" << exp << "*" << unit << endl;
		ptr = eptr;
		return 0;
	} else {
		eptr--; // discard modifier
		exp=0;
		if ( strncmp(eptr, unit.c_str(), unit.length() ) == 0 ) {	// try direct unit read
			eptr += unit.length();
			exp = 0;
			ptr = eptr;
			return 0;
		}
	}
	
	return -2;
} // end scan_unit

void value_t::setUnit(std::string Unit) throw() { unit = Unit; }
std::string value_t::Unit() throw() {
	std::ostringstream os;
	if (flags & value_t::NO_WRITE_UNIT) {
		if (exp!=0) os << "E" << exp;
	} else {
		switch (exp) {
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
		default:	os << "E" << exp;
		}
		os << unit;
	}
	return os.str();
}

String_t value_t::String() STD_VAL_THROW { return (String_t)""; }
Double_t value_t::Double() STD_VAL_THROW { return (Double_t)0; }
Integer_t value_t::Integer() STD_VAL_THROW { return (Integer_t)0; }
Bool_t value_t::Bool() STD_VAL_THROW { return (Bool_t)0; }


// operators
// arithmetric	++ -- * / % + -
sensation_t* value_t::operator_dot(sensation_t* other) STD_OP_THROW {	// string concatenation
	return new valueconstT<String_t>(this->Name(), "", this->String() + other->String());	// FIXME: exp here
}
sensation_t* value_t::operator*(sensation_t* other) STD_OP_THROW {
	return new valueconstT<Double_t>(this->Name(), this->unit, this->Double() * other->Double());	// FIXME: exp here
}
sensation_t* value_t::operator/(sensation_t* other) STD_OP_THROW {
	return new valueconstT<Double_t>(this->Name(), this->unit, this->Double() / other->Double());	// FIXME: exp here
}
sensation_t* value_t::operator%(sensation_t* other) STD_OP_THROW {
	throw(errno_exception<EINVAL>("operation not implemented"));
}
sensation_t* value_t::operator+(sensation_t* other) STD_OP_THROW {
	return new valueconstT<Double_t>(this->Name(), this->unit, this->Double() + other->Double());	// FIXME: exp here
}
sensation_t* value_t::operator-(sensation_t* other) STD_OP_THROW {
	return new valueconstT<Double_t>(this->Name(), this->unit, this->Double() - other->Double());	// FIXME: exp here
}

// compare	< > == != <= >=
sensation_t* value_t::operator<(sensation_t* other) STD_OP_THROW {
	return new valueconstT<Bool_t>(this->Name(), "", this->Double() < other->Double());
}
sensation_t* value_t::operator>(sensation_t* other) STD_OP_THROW {
	return new valueconstT<Bool_t>(this->Name(), "", this->Double() > other->Double());
}
sensation_t* value_t::operator==(sensation_t* other) STD_OP_THROW {
	return new valueconstT<Bool_t>(Name(), "", this->Double() == other->Double());
}
sensation_t* value_t::operator!=(sensation_t* other) STD_OP_THROW {
	return new valueconstT<Bool_t>(Name(), "", this->Double() != other->Double());
}
sensation_t* value_t::operator<=(sensation_t* other) STD_OP_THROW {
	return new valueconstT<Bool_t>(Name(), "", this->Double() <= other->Double());
}
sensation_t* value_t::operator>=(sensation_t* other) STD_OP_THROW {
	return new valueconstT<Bool_t>(Name(), "", this->Double() >= other->Double());
}

