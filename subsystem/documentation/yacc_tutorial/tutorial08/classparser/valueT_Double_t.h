/**
 * \file valueT_Double_t.h
 * \author Dennis Terhorst
 * \date Wed Oct 14 19:20:35 CEST 2009
 */
#ifndef VALUET_DOUBLE_T_H
#define VALUET_DOUBLE_T_H
#include "valueT.h"
#include "valueconstT.h"

// //////////////////////////////////////////////////////////////////////////////
// SPECIALISATION -- Double_t
// //////////////////////////////////////////////////////////////////////////////

#include <sstream>
template <>
class valueT<Double_t> : public value_t {
private:
	Double_t	value;
	Double_t	scanned_value;
public:
	virtual ~valueT() throw() { };
	virtual void accept() throw() { value = scanned_value; value_t::accept(); }
	virtual value_t* copy() { return new valueT<Double_t>(*this); }

	valueT(std::string Name, std::string Unit, flags_t Flags=NO_RW_UNIT) throw()
	 : value_t(Name, Unit, Flags) {
		value = 0.0;
		scanned_value = value;
	}

	template <typename T> friend std::ostream& operator<<(std::ostream& os, valueT<T>& val) throw();

	// NEEDS SPECIALISATION:
	virtual value_t::valuetype_t type() throw() { return value_t::valueD; }
	virtual int scan_value(char*& ptr) throw() {
		char* eptr=ptr;
		skip_ws(eptr);
		scanned_value = strtod(ptr, &eptr);
		if (eptr == ptr) { return -1; }
		ptr= eptr;
		return 0;
	}

	Integer_t Integer() STD_VAL_THROW {
		if (! this->hasValue() ) throw(errno_exception<ENOMSG>("not received jet"));
		return value;
	}
	Double_t Double() STD_VAL_THROW {
		if (! this->hasValue() ) throw(errno_exception<ENOMSG>("not received jet"));
		return value;
	}
	String_t String() STD_VAL_THROW {
		if (! this->hasValue() ) throw(errno_exception<ENOMSG>("not received jet"));
		std::ostringstream os(std::ostringstream::out);
		os << *this;
		return os.str();
	}
	Bool_t Bool() STD_VAL_THROW {
		if (! this->hasValue() ) throw(errno_exception<ENOMSG>("not received jet"));
		return value;
	}

	value_t* operator=(Double_t other) STD_OP_THROW {
		value = other;
		return this;
	}
}; // end class valueT<Double_t>

#endif //ndef VALUET_DOUBLE_T_H
