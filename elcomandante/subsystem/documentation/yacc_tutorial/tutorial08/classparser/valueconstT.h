/**
 * \file valueconstT.h
 * template value kind of value_t
 *
 * \author Dennis Terhorst
 * \date Wed Oct 14 12:25:33 CEST 2009
 */
#ifndef VALUECONSTT_H
#define VALUECONSTT_H

#include <iostream>
#include <string>
#include "value_t.h"

#include <math.h>
#include <stdlib.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief template of a scannable constant value_t sensation
 *
 * This pure virtual class template has to be specialized for all different
 * constant value types to be read from packets.
 *
 * Constant values read their exact value or fail. Thus if a value different to
 * their current value is received parsing fails, as opposed to non-constant
 * valueT, which will eventually succeed and take the new value.
 */
template <typename TYPE>
class valueconstT : public value_t {
private:
	TYPE		value;
	//protected from above:
	//int		exp;
	//std::string	unit;
public:
	virtual ~valueconstT() throw() { };
	virtual void accept() throw() { value_t::accept(); }
	virtual value_t* copy()=0;

	valueconstT(std::string Name, std::string Unit, TYPE Val, flags_t Flags=NOFLAG) throw()
	 : value_t(Name, Unit, Flags) {
		value = Val;
		value_t::accept();
	}

	template <typename T> friend std::ostream& operator<<(std::ostream& os, valueconstT<T>& val) throw();

	// NEEDS SPECIALISATION:
	virtual value_t::valuetype_t type() throw(); // =0
	virtual int scan_value(char*& ptr) throw(); // =0

}; // end class valueconstT<>

// GLOBAL SCOPE STREAM OUT OPERATOR
template <typename T>
std::ostream& operator<<(std::ostream& os, valueconstT<T>& val) throw() { return os << val.value << val.unit; }

// //////////////////////////////////////////////////////////////////////////////

#include "valueconstT_Bool_t.h"	// needed first for operators
#include "valueconstT_Integer_t.h"
#include "valueconstT_Double_t.h"
#include "valueconstT_String_t.h"

// //////////////////////////////////////////////////////////////////////////////

#endif //ndef VALUECONSTT_H
