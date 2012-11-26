/**
 * \file sensations.h
 * \author Dennis Terhorst
 * \date Wed Jul  8 09:53:15 CEST 2009
 *
 * Storage classes for value_t with names and value combination classes
 */
#ifndef SENSATIONS_H
#define SENSATIONS_H
////////////////////////////////////////////////////////////////////////////////
// SENSATIONS
////////////////////////////////////////////////////////////////////////////////
#include <string>
using namespace std;

#include "value_t.h"
class sensation_t {
public:
	//sensation_t() {}
	virtual ~sensation_t() {};

	virtual int scan(char*& ptr)=0;
	virtual string Name()=0;
	virtual Double_t Double()=0;
	virtual Integer_t Integer()=0;
	virtual String_t String()=0;
};

////////////////////////////////////////////////////////////////////////////////
#include "value_t.h"
class sens_value_t : public sensation_t { // FIXME: rename value_t ?!
	string name;
	value_t* value;
public:
	sens_value_t(string Name, value_t* Value) { name=Name; value=Value; }
	virtual ~sens_value_t() {};

	inline int scan(char*& ptr) { return value->scan(ptr); } // forward scanning to the underlaying value_t object.
	virtual string Name() { return name; }
	virtual Double_t Double() { return value->Double(); }
	virtual Integer_t Integer() { return value->Integer(); };
	virtual String_t String() { return value->String(); };
};

class sens_sum_t : public sensation_t {
	string name;
	sensation_t* first;
	sensation_t* second;
public:
	sens_sum_t(sensation_t* First, sensation_t* Second, string Name="SUM") {
		name = Name;
		first = First;
		second = Second;
	}
	virtual ~sens_sum_t() {};
	virtual string Name() {
		return name+"("+first->Name()+", "+second->Name()+")";
	}
	virtual int scan(char*&ptr) { return -1; }	// cannot scan complex type
	virtual Double_t Double()   { return first->Double()+second->Double(); }
	virtual Integer_t Integer() { return first->Integer()+second->Integer(); }
	virtual String_t String()   { return first->String()+second->String(); }
};

#include <iostream>
#include <sstream>
class sens_diff_t : public sensation_t {
	string name;
	sensation_t* first;
	sensation_t* second;
public:
	sens_diff_t(sensation_t* First, sensation_t* Second, string Name="DIFF") {
		name = Name;
		first = First;
		second = Second;
	}
	virtual ~sens_diff_t() {};
	virtual string Name() {
		return name+"("+first->Name()+", "+second->Name()+")";
	}
	virtual int scan(char*&ptr) { return -1; }	// cannot scan complex type
	virtual Double_t Double()   { return first->Double()-second->Double(); }
	virtual Integer_t Integer() { return first->Integer()-second->Integer(); }
	virtual String_t String()   {
		ostringstream os(ostringstream::out);
		os << (first->Double()-second->Double());
		return os.str();
	}
//	virtual String_t String()   { return String_t("undefined"); }
};

////////////////////////////////////////////////////////////////////////////////


#endif //ndef SENSATIONS_H
