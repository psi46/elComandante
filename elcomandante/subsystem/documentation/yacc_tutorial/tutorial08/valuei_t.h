/**
 * \file valuei_t.h
 * integer value kind of value_t
 *
 * \author Dennis Terhorst
 * \date So 28. Jun 15:10:51 CEST 2009
 */
#ifndef VALUEI_T_H
#define VALUEI_T_H

#include <iostream>
#include <string>
using namespace std;
#include "value_t.h"

////////////////////////////////////////////////////////////////////////////////////
class valuei_t : public value_t {
private:
	long int value;
public:
	valuei_t(string Unit, flags_t Flags=NOFLAG);
	virtual ~valuei_t() {};

	friend ostream& operator<<(ostream& os, valuei_t& val);
	// virtuals from value_t
	virtual int scan_value(char*& ptr);
	virtual String_t String();
	virtual Double_t Double();
	virtual Integer_t Integer();
}; // end class valuei_t

#endif //ndef VALUEI_T_H
