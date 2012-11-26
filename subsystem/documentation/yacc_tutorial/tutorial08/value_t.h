/**
 * \file value_t.h
 * interface for value read/write classes
 * \author Dennis Terhorst
 * \date So 28. Jun 14:52:39 CEST 2009
 *
 * NOTE:
 * These classes use "inheritance for implementation"
 * That's the bad kind of inheritance!      -- Dennis
 */
#ifndef VALUE_T_H
#define VALUE_T_H

#include <string.h>	// for strncmp()
#include <string>
#include <math.h>	// for nan()
using namespace std;

typedef string String_t;
typedef double Double_t;
typedef long int Integer_t;

////////////////////////////////////////////////////////////////////////////////////
class value_t {
public:
	enum {NOFLAG=0, NO_READ_UNIT=1, NO_WRITE_UNIT=2} flag_types;
	typedef unsigned int flags_t;
protected:
	string	unit;
	int	exp;
	flags_t flags;
private:
	bool initialized;	// set to true after frist successfull scan
public:
	value_t(string Unit="", flags_t Flags=NOFLAG) {
		unit = Unit;
		exp = 0;
		flags = Flags;
		initialized = false;
	}
	value_t(const value_t& v) {
		unit = v.unit;
		exp = 0;
		flags = v.flags;
		initialized = false;
	}
	virtual ~value_t() {};	// destructor must always be virtual!

	void setUnit(string Unit) { unit=Unit; return; }
	bool hasValue() const { return initialized; }
	/**
	 * scan value from the given ptr position
	 * return <0 if scan failed (must leave ptr unmodified)
	 *        ==0 if scan succeeded (advance ptr size of conversion)
	 */
	virtual inline int scan(char*& ptr);
	
	/**
	 * stream value in a char format which can be read back via scan()
	 */
protected:
	virtual inline void skip_ws(char*& ptr) const;
	virtual inline int scan_unit(char*& ptr);

	virtual int scan_value(char*& ptr) { return -1; };
public:
	virtual String_t String() { return string("VALUE_T::UNDEFINED"); };
	virtual Double_t Double() { return nan("not received jet"); /*0.0/0.0;*/ /*NaN*/ };
	virtual Integer_t Integer() { return 0; /*NaN*/ };
//	friend ostream& operator<<(ostream& os, value_t& val);
}; // end class value_t
////////////////////////////////////////////////////////////////////////////////////

void value_t::skip_ws(char*& ptr) const {
	while (*ptr == ' ' || *ptr == '\t' ) { ptr++; }
} // end skip_ws

int value_t::scan(char*& ptr) {
	char* oldptr=ptr;
	if ( this->scan_value(ptr) < 0 ) { ptr=oldptr; return -1; }
	if ( !(flags&NO_READ_UNIT) && (this->scan_unit(ptr) < 0) ) { ptr=oldptr; return -2; }
	initialized = true;
	return 0;
} // end scan

int value_t::scan_unit(char*& ptr) {
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
			//cout << "scanned unit " << unit << endl;
			ptr = eptr;
			return 0;
		}
	}
	
	return -2;
} // end scan_unit



#endif //ndef VALUE_T_H
