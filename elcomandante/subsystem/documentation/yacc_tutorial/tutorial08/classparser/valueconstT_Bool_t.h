/**
 * \file valueconstT_Bool_t.h
 * \author Dennis Terhorst
 * \date Wed Oct 14 19:50:03 CEST 2009
 */
#ifndef VALUECONSTT_BOOL_T_H
#define VALUECONSTT_BOOL_T_H
#include "valueconstT.h"

// //////////////////////////////////////////////////////////////////////////////
// SPECIALISATION -- Bool_t
// //////////////////////////////////////////////////////////////////////////////
template <>
class valueconstT<Bool_t> : public value_t {
private:
	Bool_t	value;
public:
	virtual ~valueconstT() throw() { };
	virtual void accept() throw() { value_t::accept(); }
	virtual value_t* copy() { return new valueconstT<Bool_t>(*this); }

	valueconstT(std::string Name, std::string Unit, Bool_t Val, flags_t Flags=NO_RW_UNIT) throw()
	 : value_t(Name, Unit, Flags) {
		value = Val;
		value_t::accept();

	}

	template <typename T> friend std::ostream& operator<<(std::ostream& os, valueconstT<T>& val) throw();

	// NEEDS SPECIALISATION:
	virtual value_t::valuetype_t type() throw() { return value_t::valueB; }
	virtual int scan_value(char*& ptr) throw() {
		return 0;		// FIXME: WRONG !!!
	}

	Integer_t Integer() STD_VAL_THROW { return value; }
	Double_t  Double()  STD_VAL_THROW { return value; }
	String_t  String() STD_VAL_THROW  { return (value?"true":"false"); }
	Bool_t    Bool()    STD_VAL_THROW { return value; }

/*
	// operators
	// arithmetric	++ -- * / % + -
	sensation_t* operator*(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case sensation_t::valueI:
		case sensation_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), "", this->Bool() * other->Bool());	// FIXME: unit here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator/(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case sensation_t::valueI:
		case sensation_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), "", this->Bool() / other->Bool());	// FIXME: unit here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator%(sensation_t* other) STD_OP_THROW {
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator+(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case sensation_t::valueI:
		case sensation_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), "", this->Bool() + other->Bool());	// FIXME: unit here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator-(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case sensation_t::valueI:
		case sensation_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), "", this->Bool() - other->Bool());	// FIXME: unit here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}

	// compare	< > == != <= >=
	sensation_t* operator<(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case sensation_t::valueI:
		case sensation_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), "", this->Bool() < other->Bool());	// FIXME: unit here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator>(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case sensation_t::valueI:
		case sensation_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), "", this->Bool() > other->Bool());	// FIXME: unit here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator==(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case sensation_t::valueI:
		case sensation_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), "", this->Bool() == other->Bool());	// FIXME: unit here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator!=(sensation_t* other) STD_OP_THROW {
		valueb_t* tmp;
		switch (other->type()) {
		case sensation_t::valueI:
		case sensation_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), "", this->Bool() != other->Bool());	// FIXME: unit here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator<=(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case sensation_t::valueI:
		case sensation_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), "", this->Bool() <= other->Bool());	// FIXME: unit here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator>=(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case sensation_t::valueI:
		case sensation_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), "", this->Bool() >= other->Bool());	// FIXME: unit here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator=(sensation_t* other) STD_OP_THROW {
		value = other->Bool();
		return this;
	}
	sensation_t* operator=(Bool_t other) STD_OP_THROW {
		value = other;
		return this;
	}
*/

}; // end class valueconstT<Bool_t>




#endif //ndef VALUECONSTT_BOOL_T_H
