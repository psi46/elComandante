/**
 * \file sens_arith_t.h
 * \author Dennis Terhorst
 * \date Mon Oct 12 17:56:25 CEST 2009
 */
#ifndef SENS_ARITH_T_H
#define SENS_ARITH_T_H

#include "sensations.h"

/** \brief Abstract class for arithmetric expressions (operators) */
class sens_operator_t : public sensation_t {
protected:
	std::string name;
	sensation_t* first;
	sensation_t* second;
public:
	sens_operator_t(sensation_t* First, sensation_t* Second, std::string Name="OP") {
		name = Name;
		first = First;
		second = Second;
	}
	virtual ~sens_operator_t() {};
	virtual sens_operator_t* copy()=0;
	virtual valuetype_t type() throw()=0;
	virtual std::string Name() {
		return name+"("+first->Name()+", "+second->Name()+")";
	}
	virtual int scan(char*&ptr)	STD_VAL_THROW { return -1; }	// cannot scan complex type
	virtual Double_t Double()	STD_VAL_THROW=0;
	virtual Integer_t Integer()	STD_VAL_THROW=0;
	virtual String_t String()	STD_VAL_THROW=0;
	virtual Bool_t Bool()		STD_VAL_THROW=0;


	// operators
	// arithmetric	++ -- * / % + -
	//	// FIXME: these definitions do not forward the exponent (exp) correctly
	//	// FIXME: these definitions do not take care of units
	// string concatenation
	sensation_t* operator_dot(sensation_t* other) STD_OP_THROW { return new valueconstT<String_t>(this->Name(), "", this->String() + other->String()); }
	sensation_t* operator*(sensation_t* other)    STD_OP_THROW { return new valueconstT<Double_t>(this->Name(), "", this->Double() * other->Double()); }
	sensation_t* operator/(sensation_t* other)    STD_OP_THROW { return new valueconstT<Double_t>(this->Name(), "", this->Double() / other->Double()); }
	sensation_t* operator%(sensation_t* other)    STD_OP_THROW { throw(errno_exception<EINVAL>("operation not implemented")); }
	sensation_t* operator+(sensation_t* other)    STD_OP_THROW { return new valueconstT<Double_t>(this->Name(), "", this->Double() + other->Double()); }
	sensation_t* operator-(sensation_t* other)    STD_OP_THROW { return new valueconstT<Double_t>(this->Name(), "", this->Double() - other->Double()); }

	// compare	< > == != <= >=
	sensation_t* operator<(sensation_t* other)  STD_OP_THROW { return new valueconstT<Bool_t>(this->Name(), "", this->Double() < other->Double()); }
	sensation_t* operator>(sensation_t* other)  STD_OP_THROW { return new valueconstT<Bool_t>(this->Name(), "", this->Double() > other->Double()); }
	sensation_t* operator==(sensation_t* other) STD_OP_THROW { return new valueconstT<Bool_t>(Name(), "", this->Double() == other->Double()); }
	sensation_t* operator!=(sensation_t* other) STD_OP_THROW { return new valueconstT<Bool_t>(Name(), "", this->Double() != other->Double()); }
	sensation_t* operator<=(sensation_t* other) STD_OP_THROW { return new valueconstT<Bool_t>(Name(), "", this->Double() <= other->Double()); }
	sensation_t* operator>=(sensation_t* other) STD_OP_THROW { return new valueconstT<Bool_t>(Name(), "", this->Double() >= other->Double()); }

};

/* /////////////////////////////////////////////////////////////////////////////
	// arithmetric	++ -- * / % + -
	virtual sensation_t* operator_dot(sensation_t* other) STD_OP_DEF(".");
	virtual sensation_t* operator*(sensation_t* other) STD_OP_DEF("*");
	virtual sensation_t* operator/(sensation_t* other) STD_OP_DEF("/");
	virtual sensation_t* operator%(sensation_t* other) STD_OP_DEF("%");
	virtual sensation_t* operator+(sensation_t* other) STD_OP_DEF("+");
	virtual sensation_t* operator-(sensation_t* other) STD_OP_DEF("-");
///////////////////////////////////////////////////////////////////////////// */

class sens_dot_t : public sens_operator_t {
public:
	sens_dot_t(sensation_t* First, sensation_t* Second, std::string Name="CONCAT") : sens_operator_t(First,Second,Name) {};
	virtual ~sens_dot_t() {};
	virtual sens_operator_t* copy()		      { return new sens_dot_t(*this); };
	virtual valuetype_t type()	throw()       { return valueS; }
	virtual Double_t Double()	STD_VAL_THROW { return (first->operator_dot(second))->Double(); }
	virtual Integer_t Integer()	STD_VAL_THROW { return (first->operator_dot(second))->Integer(); }
	virtual String_t String()	STD_VAL_THROW { return (first->operator_dot(second))->String(); }
	virtual Bool_t Bool()		STD_VAL_THROW { return (first->operator_dot(second))->Bool(); }
};
class sens_mul_t : public sens_operator_t {
public:
	sens_mul_t(sensation_t* First, sensation_t* Second, std::string Name="MUL") : sens_operator_t(First,Second,Name) {};
	virtual ~sens_mul_t() {};
	virtual sens_operator_t* copy()		      { return new sens_mul_t(*this); };
	virtual valuetype_t type()	throw()       { return valueD; }
	virtual Double_t Double()	STD_VAL_THROW { return (first->operator*(second))->Double(); }
	virtual Integer_t Integer()	STD_VAL_THROW { return (first->operator*(second))->Integer(); }
	virtual String_t String()	STD_VAL_THROW { return (first->operator*(second))->String(); }
	virtual Bool_t Bool()		STD_VAL_THROW { return (first->operator*(second))->Bool(); }
};
class sens_div_t : public sens_operator_t {
public:
	sens_div_t(sensation_t* First, sensation_t* Second, std::string Name="DIV") : sens_operator_t(First,Second,Name) {};
	virtual ~sens_div_t() {};
	virtual sens_operator_t* copy()		      { return new sens_div_t(*this); };
	virtual valuetype_t type()	throw()       { return valueD; }
	virtual Double_t Double()	STD_VAL_THROW { return (first->operator/(second))->Double(); }
	virtual Integer_t Integer()	STD_VAL_THROW { return (first->operator/(second))->Integer(); }
	virtual String_t String()	STD_VAL_THROW { return (first->operator/(second))->String(); }
	virtual Bool_t Bool()		STD_VAL_THROW { return (first->operator/(second))->Bool(); }
};
class sens_mod_t : public sens_operator_t {
public:
	sens_mod_t(sensation_t* First, sensation_t* Second, std::string Name="MOD") : sens_operator_t(First,Second,Name) {};
	virtual ~sens_mod_t() {};
	virtual sens_operator_t* copy()		      { return new sens_mod_t(*this); };
	virtual valuetype_t type()	throw()       { return valueI; }
	virtual Double_t Double()	STD_VAL_THROW { return (first->operator%(second))->Double(); }
	virtual Integer_t Integer()	STD_VAL_THROW { return (first->operator%(second))->Integer(); }
	virtual String_t String()	STD_VAL_THROW { return (first->operator%(second))->String(); }
	virtual Bool_t Bool()		STD_VAL_THROW { return (first->operator%(second))->Bool(); }
};
class sens_add_t : public sens_operator_t {
public:
	sens_add_t(sensation_t* First, sensation_t* Second, std::string Name="ADD") : sens_operator_t(First,Second,Name) {};
	virtual ~sens_add_t() {};
	virtual sens_operator_t* copy()		      { return new sens_add_t(*this); };
	virtual valuetype_t type()	throw()       { return valueD; }
	virtual Double_t Double()	STD_VAL_THROW { return (first->operator+(second))->Double(); }
	virtual Integer_t Integer()	STD_VAL_THROW { return (first->operator+(second))->Integer(); }
	virtual String_t String()	STD_VAL_THROW { return (first->operator+(second))->String(); }
	virtual Bool_t Bool()		STD_VAL_THROW { return (first->operator+(second))->Bool(); }
};
class sens_diff_t : public sens_operator_t {
public:
	sens_diff_t(sensation_t* First, sensation_t* Second, std::string Name="DIFF") : sens_operator_t(First,Second,Name) {};
	virtual ~sens_diff_t() {};
	virtual sens_operator_t* copy()		      { return new sens_diff_t(*this); };
	virtual valuetype_t type()	throw()       { return valueD; }
	virtual Double_t Double()	STD_VAL_THROW { return (first->operator-(second))->Double(); }
	virtual Integer_t Integer()	STD_VAL_THROW { return (first->operator-(second))->Integer(); }
	virtual String_t String()	STD_VAL_THROW { return (first->operator-(second))->String(); }
	virtual Bool_t Bool()		STD_VAL_THROW { return (first->operator-(second))->Bool(); }
};


/* /////////////////////////////////////////////////////////////////////////////
	// compare	< > == != <= >=
	virtual sensation_t* operator<(sensation_t* other) STD_OP_DEF("<");
	virtual sensation_t* operator>(sensation_t* other) STD_OP_DEF(">");
	virtual sensation_t* operator==(sensation_t* other) STD_OP_DEF("==");
	virtual sensation_t* operator!=(sensation_t* other) STD_OP_DEF("!=");
	virtual sensation_t* operator<=(sensation_t* other) STD_OP_DEF("<=");
	virtual sensation_t* operator>=(sensation_t* other) STD_OP_DEF(">=");
///////////////////////////////////////////////////////////////////////////// */

class sens_gt_t : public sens_operator_t {
public:
	sens_gt_t(sensation_t* First, sensation_t* Second, std::string Name="GT") : sens_operator_t(First,Second,Name) {};
	virtual ~sens_gt_t() {};
	virtual sens_operator_t* copy()		      { return new sens_gt_t(*this); };
	virtual valuetype_t type()	throw()       { return valueD; }
	virtual Double_t Double()	STD_VAL_THROW { return (first->operator>(second))->Double(); }
	virtual Integer_t Integer()	STD_VAL_THROW { return (first->operator>(second))->Integer(); }
	virtual String_t String()	STD_VAL_THROW { return (first->operator>(second))->String(); }
	virtual Bool_t Bool()		STD_VAL_THROW { return (first->operator>(second))->Bool(); }
};
class sens_lt_t : public sens_operator_t {
public:
	sens_lt_t(sensation_t* First, sensation_t* Second, std::string Name="LT") : sens_operator_t(First,Second,Name) {};
	virtual ~sens_lt_t() {};
	virtual sens_operator_t* copy()		      { return new sens_lt_t(*this); };
	virtual valuetype_t type()	throw()       { return valueD; }
	virtual Double_t Double()	STD_VAL_THROW { return (first->operator<(second))->Double(); }
	virtual Integer_t Integer()	STD_VAL_THROW { return (first->operator<(second))->Integer(); }
	virtual String_t String()	STD_VAL_THROW { return (first->operator<(second))->String(); }
	virtual Bool_t Bool()		STD_VAL_THROW { return (first->operator<(second))->Bool(); }
};
class sens_eq_t : public sens_operator_t {
public:
	sens_eq_t(sensation_t* First, sensation_t* Second, std::string Name="EQ") : sens_operator_t(First,Second,Name) {};
	virtual ~sens_eq_t() {};
	virtual sens_operator_t* copy()		      { return new sens_eq_t(*this); };
	virtual valuetype_t type()	throw()       { return valueB; }
	virtual Double_t Double()	STD_VAL_THROW { return (first->operator==(second))->Double(); }
	virtual Integer_t Integer()	STD_VAL_THROW { return (first->operator==(second))->Integer(); }
	virtual String_t String()	STD_VAL_THROW { return (first->operator==(second))->String(); }
	virtual Bool_t Bool()		STD_VAL_THROW { return (first->operator==(second))->Bool(); }
};
class sens_neq_t : public sens_operator_t {
public:
	sens_neq_t(sensation_t* First, sensation_t* Second, std::string Name="NEQ") : sens_operator_t(First,Second,Name) {};
	virtual ~sens_neq_t() {};
	virtual sens_operator_t* copy()		      { return new sens_neq_t(*this); };
	virtual valuetype_t type()	throw()       { return valueB; }
	virtual Double_t Double()	STD_VAL_THROW { return (first->operator!=(second))->Double(); }
	virtual Integer_t Integer()	STD_VAL_THROW { return (first->operator!=(second))->Integer(); }
	virtual String_t String()	STD_VAL_THROW { return (first->operator!=(second))->String(); }
	virtual Bool_t Bool()		STD_VAL_THROW { return (first->operator!=(second))->Bool(); }
};
class sens_lteq_t : public sens_operator_t {
public:
	sens_lteq_t(sensation_t* First, sensation_t* Second, std::string Name="LTEQ") : sens_operator_t(First,Second,Name) {};
	virtual ~sens_lteq_t() {};
	virtual sens_operator_t* copy()		      { return new sens_lteq_t(*this); };
	virtual valuetype_t type()	throw()       { return valueB; }
	virtual Double_t Double()	STD_VAL_THROW { return (first->operator<=(second))->Double(); }
	virtual Integer_t Integer()	STD_VAL_THROW { return (first->operator<=(second))->Integer(); }
	virtual String_t String()	STD_VAL_THROW { return (first->operator<=(second))->String(); }
	virtual Bool_t Bool()		STD_VAL_THROW { return (first->operator<=(second))->Bool(); }
};
class sens_gteq_t : public sens_operator_t {
public:
	sens_gteq_t(sensation_t* First, sensation_t* Second, std::string Name="GTEQ") : sens_operator_t(First,Second,Name) {};
	virtual ~sens_gteq_t() {};
	virtual sens_operator_t* copy()		      { return new sens_gteq_t(*this); };
	virtual valuetype_t type()	throw()       { return valueB; }
	virtual Double_t Double()	STD_VAL_THROW { return (first->operator>=(second))->Double(); }
	virtual Integer_t Integer()	STD_VAL_THROW { return (first->operator>=(second))->Integer(); }
	virtual String_t String()	STD_VAL_THROW { return (first->operator>=(second))->String(); }
	virtual Bool_t Bool()		STD_VAL_THROW { return (first->operator>=(second))->Bool(); }
};

#endif //ndef SENS_ARITH_T_H
