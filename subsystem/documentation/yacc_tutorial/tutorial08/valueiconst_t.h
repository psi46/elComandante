/**
 * \file valueiconst_t.h
 * double value kind of value_t
 *
 * \author Dennis Terhorst
 * \date So 28. Jun 14:49:35 CEST 2009
 */
#ifndef VALUEDCONST_T_H
#define VALUEDCONST_T_H

#include <iostream>
#include <string>
using namespace std;
#include "value_t.h"

////////////////////////////////////////////////////////////////////////////////////
class valueiconst_t : public value_t {
private:
	Integer_t	value;
public:
	valueiconst_t(string Unit, Integer_t Value, flags_t Flags=value_t::NO_WRITE_UNIT);
	virtual ~valueiconst_t() {};

	friend ostream& operator<<(ostream& os, valueiconst_t& val);
	// virtuals from value_t
	virtual int scan_value(char*& ptr);
	virtual String_t String();
	virtual Double_t Double();
	virtual Integer_t Integer();
}; // end class valueiconst_t

#endif //ndef VALUEDCONST_T_H
