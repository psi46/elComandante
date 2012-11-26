/**
 * \file valueT_Integer_t.h
 * \author Dennis Terhorst
 * \date Wed Oct 14 19:20:35 CEST 2009
 */
#ifndef VALUET_INTEGER_T_H
#define VALUET_INTEGER_T_H
#include "valueT.h"

// //////////////////////////////////////////////////////////////////////////////
// SPECIALISATION -- Integer_t
// //////////////////////////////////////////////////////////////////////////////

#include <sstream>
template <>
class valueT<Integer_t> : public value_t {
private:
	Integer_t	value;
	Integer_t	scanned_value;
public:
	virtual ~valueT() throw() { };
	virtual void accept() throw() { value = scanned_value; value_t::accept(); }
	virtual value_t* copy() { return new valueT<Integer_t>(*this); }

	valueT(std::string Name, std::string Unit, flags_t Flags=NO_RW_UNIT) throw()
	 : value_t(Name, Unit, Flags) {
		value = 0;
		scanned_value = value;
	}

	template <typename T> friend std::ostream& operator<<(std::ostream& os, valueT<T>& val) throw();

	// NEEDS SPECIALISATION:
	virtual value_t::valuetype_t type() throw() { return value_t::valueI; }
	virtual int scan_value(char*& ptr) throw() {
		char* eptr=ptr;
		skip_ws(eptr);
		scanned_value = strtol(ptr, &eptr, 0);
		if (eptr == ptr) { return -1; }	// no valid input at all
		ptr = eptr;
		return 0;
	}

	Integer_t Integer() STD_VAL_THROW {
		if (! this->hasValue() ) throw(errno_exception<ENOMSG>("not received jet"));
		return value;
	}
	Double_t Double() STD_VAL_THROW {
		if (! this->hasValue() ) throw(errno_exception<ENOMSG>("not received jet"));
		return (Double_t)value;
	}
	String_t String() STD_VAL_THROW {
		std::ostringstream os(std::ostringstream::out);
		os << *this;
		return os.str();
	}
	Bool_t Bool() STD_VAL_THROW {
		if (! this->hasValue() ) throw(errno_exception<ENOMSG>("not received jet"));
		return (Bool_t)value;
	}
}; // end class valueT<>


#endif //ndef VALUET_INTEGER_T_H
