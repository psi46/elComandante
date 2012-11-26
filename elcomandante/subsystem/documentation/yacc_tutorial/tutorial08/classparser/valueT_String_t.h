/**
 * \file valueT_String_t.h
 * \author Dennis Terhorst
 * \date Wed Oct 14 19:20:35 CEST 2009
 */
#ifndef VALUET_STRING_T_H
#define VALUET_STRING_T_H
#include "valueT.h"

// //////////////////////////////////////////////////////////////////////////////
// SPECIALISATION -- String_t
// //////////////////////////////////////////////////////////////////////////////

#include <sstream>
template <>
class valueT<String_t> : public value_t {
private:
	String_t	value;
	String_t	scanned_value;
public:
	virtual ~valueT() throw() { };
	virtual void accept() throw() { value = scanned_value; value_t::accept(); }
	virtual value_t* copy() { return new valueT<String_t>(*this); }

	valueT(std::string Name, std::string Unit, flags_t Flags=NO_RW_UNIT) throw()
	 : value_t(Name, Unit, Flags) {
		value = "UNDEFINED";
		scanned_value = value;
	}
	template <typename T> friend std::ostream& operator<<(std::ostream& os, valueT<T>& val) throw();

	// NEEDS SPECIALISATION:
	virtual value_t::valuetype_t type() throw() { return value_t::valueS; }
	virtual int scan_value(char*& ptr) throw() {
		char* eptr = ptr;
		skip_ws(eptr);
		std::istringstream is(eptr);
		is >> scanned_value;
		if ( scanned_value.size() > 0 ) {
			ptr=eptr+scanned_value.size();
			return 0;
		}
		return -1;
	}

	Integer_t Integer() STD_VAL_THROW { return 0; }
	Double_t Double() STD_VAL_THROW   { return 0.0; }
	String_t  String()  STD_VAL_THROW { return value; }
	Bool_t    Bool()    STD_VAL_THROW { return value.compare(""); }

}; // end class valueT<>


#endif //ndef VALUET_STRING_T_H
