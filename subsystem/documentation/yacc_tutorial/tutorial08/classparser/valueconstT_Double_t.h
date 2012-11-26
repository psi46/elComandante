/**
 * \file valueconstT_Double_t.h
 * \author Dennis Terhorst
 * \date Wed Oct 14 19:50:03 CEST 2009
 */
#ifndef VALUECONSTT_DOUBLE_T_H
#define VALUECONSTT_DOUBLE_T_H
#include "valueconstT.h"

// //////////////////////////////////////////////////////////////////////////////
// SPECIALISATION -- Double_t
// //////////////////////////////////////////////////////////////////////////////
template <>
class valueconstT<Double_t> : public value_t {
private:
	Double_t	value;
public:
	virtual ~valueconstT() throw() { };
	virtual void accept() throw() { value_t::accept(); }
	virtual value_t* copy() { return new valueconstT<Double_t>(*this); }

	valueconstT(std::string Name, std::string Unit, Double_t Val, flags_t Flags=NO_RW_UNIT) throw()
	 : value_t(Name, Unit, Flags) {
		value = Val;
		value_t::accept();
	}

	template <typename T> friend std::ostream& operator<<(std::ostream& os, valueconstT<T>& val) throw();

	// NEEDS SPECIALISATION:
	virtual value_t::valuetype_t type() throw() { return value_t::valueD; }
	virtual int scan_value(char*& ptr) throw() {
		char* eptr=ptr;
		skip_ws(eptr);
		double readval = strtod(ptr, &eptr);
		if (eptr == ptr) { return -1; }
		if (readval != value) { return -1; }
		ptr= eptr;
		return 0;
	}

	Integer_t Integer() STD_VAL_THROW { return value; }
	Double_t  Double()  STD_VAL_THROW { return value; }
	String_t String() STD_VAL_THROW {
		std::ostringstream os(std::ostringstream::out);
		os << *this;
		return os.str();
	}
	Bool_t    Bool()    STD_VAL_THROW { return value; }
/*
	// operators
	// arithmetric	++ -- * / % + -
	sensation_t* operator*(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case value_t::valueI:
		case value_t::valueD:
			return new valueconstT<Double_t>(this->Name(), this->unit, this->Double() * other->Double());	// FIXME: exp here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator/(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case value_t::valueI:
		case value_t::valueD:
			return new valueconstT<Double_t>(this->Name(), this->unit, this->Double() / other->Double());	// FIXME: exp here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator+(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case value_t::valueI:
		case value_t::valueD:
			return new valueconstT<Double_t>(this->Name(), this->unit, this->Double() + other->Double());	// FIXME: exp here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator-(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case value_t::valueI:
		case value_t::valueD:
			return new valueconstT<Double_t>(this->Name(), this->unit, this->Double() - other->Double());	// FIXME: exp here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}

	// compare	< > == != <= >=
	sensation_t* operator<(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case value_t::valueI:
		case value_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), this->unit, this->Double() < other->Double());	// FIXME: exp here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator>(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case value_t::valueI:
		case value_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), this->unit, this->Double() > other->Double());	// FIXME: exp here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator==(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case value_t::valueI:
		case value_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), this->unit, this->Double() == other->Double());	// FIXME: exp here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator!=(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case value_t::valueI:
		case value_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), this->unit, this->Double() != other->Double());	// FIXME: exp here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator<=(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case value_t::valueI:
		case value_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), this->unit, this->Double() <= other->Double());	// FIXME: exp here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}
	sensation_t* operator>=(sensation_t* other) STD_OP_THROW {
		switch (other->type()) {
		case value_t::valueI:
		case value_t::valueD:
			return new valueconstT<Bool_t>(this->Name(), this->unit, this->Double() >= other->Double());	// FIXME: exp here
		}
		throw(errno_exception<EINVAL>("operation not implemented"));
	}

	sensation_t* operator=(Double_t other) STD_OP_THROW {
		value = other;
		return this;
	}
*/
}; // end class valueconstT<>

#endif //ndef VALUECONSTT_DOUBLE_T_H
