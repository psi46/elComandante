/**
 * \file valueconstT_Integer_t.h
 * \author Dennis Terhorst
 * \date Wed Oct 14 19:48:13 CEST 2009
 */
#ifndef VALUECONSTT_INTEGER_T_H
#define VALUECONSTT_INTEGER_T_H
#include "valueconstT.h"

// //////////////////////////////////////////////////////////////////////////////
// SPECIALISATION -- Integer_t
// //////////////////////////////////////////////////////////////////////////////
template <>
class valueconstT<Integer_t> : public value_t {
private:
	Integer_t	value;
public:
	virtual ~valueconstT() throw() { };
	virtual void accept() throw() { value_t::accept(); }
	virtual value_t* copy() { return new valueconstT<Integer_t>(*this); }

	valueconstT(std::string Name, std::string Unit, Integer_t Val, flags_t Flags=NO_RW_UNIT) throw()
	 : value_t(Name, Unit, Flags) {
		value = Val;
		value_t::accept();
	}

	template <typename T> friend std::ostream& operator<<(std::ostream& os, valueconstT<T>& val) throw();

	// NEEDS SPECIALISATION:
	virtual value_t::valuetype_t type() throw() { return value_t::valueI; }
	virtual int scan_value(char*& ptr) throw() {
		char* eptr=ptr;
		skip_ws(eptr);
		Integer_t readval = strtol(ptr, &eptr, 0);
		if (eptr == ptr) { return -1; }
		if (readval != value) { return -1; }
		ptr= eptr;
		return 0;
	}

	Integer_t Integer() STD_VAL_THROW { return value; }
	Double_t  Double()  STD_VAL_THROW { return value; }
	String_t  String() STD_VAL_THROW {
		std::ostringstream os(std::ostringstream::out);
		os << *this;
		return os.str();
	}
	Bool_t    Bool()    STD_VAL_THROW { return value; }
}; // end class valueconstT<>

#endif //ndef VALUECONSTT_INTEGER_T_H

