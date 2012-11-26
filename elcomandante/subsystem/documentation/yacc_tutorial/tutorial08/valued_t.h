/**
 * \file valued_t.h
 * double value kind of value_t
 *
 * \author Dennis Terhorst
 * \date So 28. Jun 14:49:35 CEST 2009
 */
#ifndef VALUED_T_H
#define VALUED_T_H

#include <iostream>
#include <string>
using namespace std;
#include "value_t.h"

////////////////////////////////////////////////////////////////////////////////////
class valued_t : public value_t {
private:
	double	value;
public:
	valued_t(string Unit, flags_t Flags=NOFLAG);
	virtual ~valued_t() {};

	friend ostream& operator<<(ostream& os, valued_t& val);
	// virtuals from value_t
	virtual int scan_value(char*& ptr);
	virtual String_t String();
	virtual Double_t Double();
	virtual Integer_t Integer();
}; // end class valued_t

#endif //ndef VALUED_T_H
