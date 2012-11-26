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
#include <typeinfo>
#include "exceptions.h"

typedef std::string String_t;
typedef double Double_t;
typedef long int Integer_t;
typedef bool Bool_t;

#define STD_VAL_THROW throw(errno_exception<ENOMSG>)
#define STD_OP_THROW throw(errno_exception<ENOMSG>, errno_exception<EINVAL>)
#define STD_OP_DEF(op) STD_OP_THROW {\
		  throw(errno_exception<EINVAL>(\
		             	std::string("operation not implemented: '")\
				 + typeid(*this).name()\
				 + " "+ op + " "\
				 + typeid(*other).name()+"'"\
			)\
		  );\
 }

/** \brief abstract class for values and expressions
 *
 * Anything that can be perceived by the system (values, arithmetric
 * expressions, math functions, ...) is derived from this class. This
 * establishes the concept of expressions, which can be used in arbritrarily
 * nested forms.
 */
class sensation_t {
public:
	//sensation_t() {}
	virtual ~sensation_t() {};
	virtual sensation_t* copy()=0; // copy of most derived object

	typedef enum {valueD, valueS, valueI, valueB, valueT } valuetype_t;
	virtual valuetype_t type()=0;
	
	virtual int scan(char*& ptr)=0;

	virtual std::string Name()=0;
	virtual Double_t Double() STD_VAL_THROW =0;
	virtual Integer_t Integer() STD_VAL_THROW =0;
	virtual String_t String() STD_VAL_THROW =0;
	virtual Bool_t Bool() STD_VAL_THROW =0;

	// arithmetric	++ -- * / % + -
	virtual sensation_t* operator_dot(sensation_t* other) STD_OP_DEF(".");
	virtual sensation_t* operator*(sensation_t* other) STD_OP_DEF("*");
	virtual sensation_t* operator/(sensation_t* other) STD_OP_DEF("/");
	virtual sensation_t* operator%(sensation_t* other) STD_OP_DEF("%");
	virtual sensation_t* operator+(sensation_t* other) STD_OP_DEF("+");
	virtual sensation_t* operator-(sensation_t* other) STD_OP_DEF("-");

	// compare	< > == != <= >=
	virtual sensation_t* operator<(sensation_t* other) STD_OP_DEF("<");
	virtual sensation_t* operator>(sensation_t* other) STD_OP_DEF(">");
	virtual sensation_t* operator==(sensation_t* other) STD_OP_DEF("==");
	virtual sensation_t* operator!=(sensation_t* other) STD_OP_DEF("!=");
	virtual sensation_t* operator<=(sensation_t* other) STD_OP_DEF("<=");
	virtual sensation_t* operator>=(sensation_t* other) STD_OP_DEF(">=");

	// array/ptr	() [] ->
	// ...
	// binary	~ << >> & ^ |
	// ...
	// logic  	! && ||
	// ...
	
}; // end class sensation_t

#endif //ndef SENSATIONS_H
