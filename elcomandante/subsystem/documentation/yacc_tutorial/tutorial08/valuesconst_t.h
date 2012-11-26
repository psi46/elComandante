/**
 * \file valuesconst_t.h
 * string value kind of value_t
 *
 * \author Dennis Terhorst
 * \date So 28. Jun 14:54:47 CEST 2009
 */
#ifndef VALUES_T_H
#define VALUES_T_H

#include <iostream>
#include <string>
#include "value_t.h"
using namespace std;

////////////////////////////////////////////////////////////////////////////////////
class valuesconst_t : public value_t {
////////////////////////////////////////////////////////////////////////////////////
private:
	String_t value;
public:
	valuesconst_t(String_t Val);
	virtual ~valuesconst_t() {};

	friend ostream& operator<<(ostream& os, valuesconst_t& val);
	// virtuals from value_t
	virtual int scan_value(char*& ptr);
	virtual String_t String();
	virtual Double_t Double();
	virtual Integer_t Integer();
}; // end class valuesconst_t

#endif //ndef VALUES_T_H
