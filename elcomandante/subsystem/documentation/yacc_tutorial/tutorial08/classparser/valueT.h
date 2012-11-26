/**
 * \file valueT.h
 * template value kind of value_t
 *
 * \author Dennis Terhorst
 * \date Wed Oct 14 10:04:12 CEST 2009
 */
#ifndef VALUET_H
#define VALUET_H

#include <string>
#include "value_t.h"

#include <math.h>
#include <stdlib.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief template of a scannable value_t sensation
 *
 * This pure virtual class template has to be specialized for all different
 * value types to be read from packets.
 */
template <typename TYPE>
class valueT : public value_t {
private:
	TYPE		value;
	TYPE		scanned_value;
	//protected from above:
	// int		exp;
	// std::string	unit;
public:
	virtual ~valueT() throw() { };
	virtual void accept() throw() { value = scanned_value; value_t::accept(); }
	virtual value_t* copy()=0;

	valueT(std::string Name, std::string Unit, flags_t Flags=NOFLAG) throw()
	 : value_t(Name, Unit, Flags) {
		value = (TYPE)0;
		scanned_value = value;
	}

	template <typename T> friend std::ostream& operator<<(std::ostream& os, valueT<T>& val) throw();

	// NEEDS SPECIALISATION:
	virtual value_t::valuetype_t type() throw(); // =0
	virtual int scan_value(char*& ptr) throw() { return -1; } // =0

}; // end class valueT<>

// GLOBAL SCOPE STREAM OUT OPERATOR
template <typename T>
std::ostream& operator<<(std::ostream& os, valueT<T>& val) throw() { return os << val.value << val.unit; }

// //////////////////////////////////////////////////////////////////////////////

#include "valueT_Integer_t.h"
#include "valueT_Double_t.h"
#include "valueT_String_t.h"
#include "valueT_Bool_t.h"

// //////////////////////////////////////////////////////////////////////////////

#endif //ndef VALUET_H
