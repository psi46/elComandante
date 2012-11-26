/**
 * \file valueT_Bool_t.h
 * \author Dennis Terhorst
 * \date Wed Oct 14 19:20:35 CEST 2009
 */
#ifndef VALUET_BOOL_T_H
#define VALUET_BOOL_T_H
#include "valueT.h"

// //////////////////////////////////////////////////////////////////////////////
// SPECIALISATION -- Bool_t
// //////////////////////////////////////////////////////////////////////////////

#include <sstream>
template <>
class valueT<Bool_t> : public value_t {
private:
	Bool_t	value;
	Bool_t	scanned_value;
public:
	virtual ~valueT() throw() { };
	virtual void accept() throw() { value = scanned_value; value_t::accept(); }
	virtual value_t* copy() { return new valueT<Bool_t>(*this); }

	valueT(std::string Name, std::string Unit, flags_t Flags=NO_RW_UNIT) throw()
	 : value_t(Name, Unit, Flags) {
		value = false;
		scanned_value = value;
	}

	template <typename T> friend std::ostream& operator<<(std::ostream& os, valueT<T>& val) throw();

	// NEEDS SPECIALISATION:
	virtual value_t::valuetype_t type() throw() { return value_t::valueI; }
	virtual int scan_value(char*& ptr) throw() {
		char* eptr=ptr;
		skip_ws(eptr);
		scanned_value = strtod(ptr, &eptr);	// FIXME: This is wrong!!
		if (eptr == ptr) { return -1; }
		ptr= eptr;
		return 0;
	}

	Integer_t Integer() STD_VAL_THROW {
		if (! this->hasValue() ) throw(errno_exception<ENOMSG>("not received jet"));
		return (Integer_t)value;
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


#endif //ndef VALUET_BOOL_T_H
