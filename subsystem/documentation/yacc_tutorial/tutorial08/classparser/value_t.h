/**
 * \file value_t.h
 * interface for value read/write classes
 * \author Dennis Terhorst
 * \date So 28. Jun 14:52:39 CEST 2009
 *
 * NOTE:
 * These classes use "inheritance for implementation"
 * That's the bad kind of inheritance!      -- Dennis
 */
#ifndef VALUE_T_H
#define VALUE_T_H

#include <string.h>	// for strncmp()
#include <string>
#include <math.h>	// for nan()
#include "sens_value_t.h"
#include <sstream>

////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief generic value type, to be derived from by valueT and valueconstT
 *
 * This class is pure virtual and only serves the purpose to reduce code
 * duplication in the valueT template specialisations.
 */
class value_t : public sens_value_t {
public:
	enum {NOFLAG=0, NO_READ_UNIT=1, NO_WRITE_UNIT=2, NO_RW_UNIT=3} flag_types;
	typedef unsigned int flags_t;
protected:
	flags_t 	flags;
	int		exp;
	std::string	unit;
private:
	bool initialized;	// set to true after frist successfull scan
public:
	value_t(std::string name, std::string Unit="", flags_t Flags=NO_RW_UNIT) throw();
	//value_t(string name, const value_t& v) throw();
	virtual ~value_t() throw();	// destructor must always be virtual!

	/**
	 * create a copy of the most derived object
	 */
	virtual sens_value_t* copy()=0;

	bool hasValue() const throw();
	/**	 * scan value from the given ptr position without actually taking it.
	 * use following accept() function to take last scanned value.
	 *
	 * The scan function itself seldom needs to be overwritten by a
	 * derived class, as it only calls scan_value() and scan_unit(). Of
	 * those probably only scan_value() needs to be overwritten in
	 * a derived class.
	 *
	 * \return <0 if scan failed (must leave ptr unmodified)
	 *        ==0 if scan succeeded (advance ptr size of conversion)
	 */
	int scan(char*& ptr) throw();

	/**
	 * \brief accept a scanned value as new value
	 *
	 * a call to this function copies the internal scan buffer to
	 * the current value returned by the value functions.
	 */
	virtual void accept() throw();

	virtual valuetype_t type() throw() = 0;

	template <typename T>
	friend std::ostream& operator<<(std::ostream& os, value_t& val) throw(); // NEEDS SPECIALISATION!

	
protected:
	virtual void skip_ws(char*& ptr) const throw();
	virtual int scan_unit(char*& ptr) throw();
	virtual int scan_value(char*& ptr) throw()=0;
	virtual void setUnit(std::string Unit) throw();
	virtual std::string Unit() throw();
public:
	/**
	 * \name Value Return Functions
	 * These functions return return the value expressed in the requested type.
	 * Most value types will have template specialisation here!
	 */
	///@{
	virtual String_t String() STD_VAL_THROW;
	virtual Double_t Double() STD_VAL_THROW;
	virtual Integer_t Integer() STD_VAL_THROW;
	virtual Bool_t Bool() STD_VAL_THROW;
	///@}

	/**
	 * stream value in a char format which can be read back via scan()
	 */
	//there is nothing like a "virtual friend"
	//virtual friend ostream& operator<<(ostream& os, value_t& val)=0;
	
	
	/**
	 * \name Arithmetric Operators
	 * These operators need specialisation
	 */
	///@{
	// arithmetric	++ -- * / % + -
	virtual sensation_t* operator_dot(sensation_t* other) STD_OP_THROW;	// string concatenation
	virtual sensation_t* operator*(sensation_t* other) STD_OP_THROW;
	virtual sensation_t* operator/(sensation_t* other) STD_OP_THROW;
	virtual sensation_t* operator%(sensation_t* other) STD_OP_THROW;
	virtual sensation_t* operator+(sensation_t* other) STD_OP_THROW;
	virtual sensation_t* operator-(sensation_t* other) STD_OP_THROW;
	///@}

	/**
	 * \name Comparison Operators
	 */
	///@{
	// compare	< > == != <= >=
	virtual sensation_t* operator<(sensation_t* other) STD_OP_THROW;
	virtual sensation_t* operator>(sensation_t* other) STD_OP_THROW;
	virtual sensation_t* operator==(sensation_t* other) STD_OP_THROW;
	virtual sensation_t* operator!=(sensation_t* other) STD_OP_THROW;
	virtual sensation_t* operator<=(sensation_t* other) STD_OP_THROW;
	virtual sensation_t* operator>=(sensation_t* other) STD_OP_THROW;
	///@}
	
	/// \name Assignment Operators
	///@{
	// assignment
	//virtual sensation_t* operator=(sensation_t* other) STD_OP_THROW;
	///@}

}; // end class value_t
////////////////////////////////////////////////////////////////////////////////////

#include "valueT.h"
#include "valueconstT.h"
//#include "valuedconst_t.h"
//#include "valued_t.h"
//#include "valueiconst_t.h"
//#include "valuei_t.h"
//#include "valuesconst_t.h"
//#include "valueb_t.h"

#endif //ndef VALUE_T_H
