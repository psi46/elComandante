/**
 * \file valueconstT_String_t.h
 * \author Dennis Terhorst
 * \date Wed Oct 14 19:50:03 CEST 2009
 */
#ifndef VALUECONSTT_STRING_T_H
#define VALUECONSTT_STRING_T_H
#include "valueconstT.h"

// //////////////////////////////////////////////////////////////////////////////
// SPECIALISATION -- String_t
// //////////////////////////////////////////////////////////////////////////////
template <>
class valueconstT<String_t> : public value_t {
private:
	String_t	value;

	String_t doescapes() {
		String_t evalue;
		int escape=0;
		for (String_t::iterator c=value.begin(); c!=value.end(); ++c) {
			if (escape) {
				switch (*c) {
				case 'n': evalue+='\n'; break;
				case 't': evalue+='\t'; break;
				case 'r': evalue+='\r'; break;
				default:  evalue+=*c;   break;
				}
				escape=0;
				continue;
			}
			if (*c == '\\') { escape++; continue; }
			evalue+=*c;
		}
		return evalue;
	}
public:
	virtual ~valueconstT() throw() { };
	virtual void accept() throw() { value_t::accept(); }
	virtual value_t* copy() { return new valueconstT<String_t>(*this); }

	valueconstT(std::string Name, std::string Unit="", String_t Val="", flags_t Flags=NO_RW_UNIT) throw()
	 : value_t(Name, Unit, Flags) {
		value = Val;
		value_t::accept();
	}

	template <typename T> friend std::ostream& operator<<(std::ostream& os, valueconstT<T>& val) throw();

	// NEEDS SPECIALISATION:
	virtual value_t::valuetype_t type() throw() { return value_t::valueS; }
	virtual int scan_value(char*& ptr) throw() {
		String_t evalue = doescapes();	// FIXME: this is probably slowing down things coniserably. doescapes on all packet rx...!?
		char* eptr=ptr;
		skip_ws(eptr);
		int ret;
		if ( (ret=evalue.compare(0, evalue.size(), eptr, evalue.size())) == 0 ) { eptr+=evalue.size(); ptr=eptr; return 0; }
		return -1;
	}

	Integer_t Integer() STD_VAL_THROW { return 0; }		// FIXME maybe return strtol(,0) here??!
	Double_t  Double()  STD_VAL_THROW { return 0.0; }	// FIXME maybe return strtod() here?!?
	String_t  String()  STD_VAL_THROW { return doescapes(); }
	Bool_t    Bool()    STD_VAL_THROW { return value.compare(""); }

/*	sensation_t* operator==(sensation_t* other) STD_OP_THROW {
		return new valueconstT<Bool_t>(Name(), "", this->String() == other->String());
	}
	sensation_t* operator!=(sensation_t* other) STD_OP_THROW {
		return new valueconstT<Bool_t>(Name(), "", this->String() != other->String());
	}*/

}; // end class valueconstT<>
/*
template <>
ostream& operator<<(ostream& os, valueconstT<String_t>& val) throw() {
	os << val.value;
}
*/
#endif //ndef VALUECONSTT_STRING_T_H
