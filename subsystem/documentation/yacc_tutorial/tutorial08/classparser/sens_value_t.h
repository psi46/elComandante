/**
 * \file sens_value_t.h
 * \author Dennis Terhorst
 * \date Mon Oct 12 17:54:21 CEST 2009
 */
#ifndef SENS_VALUE_T_H
#define SENS_VALUE_T_H

#include "sensations.h"
#include <vector>

/** \brief Abstract class for values */
class sens_value_t : public sensation_t {
	std::string name;
public:
	sens_value_t(std::string Name) { name=Name; }
	virtual ~sens_value_t() {};
	virtual sens_value_t* copy()=0;

	virtual valuetype_t type()=0;

	virtual int scan(char*& ptr)=0;
	virtual void accept()=0;
	virtual std::string Name() { return name; }
	virtual Double_t Double() STD_VAL_THROW =0;
	virtual Integer_t Integer() STD_VAL_THROW =0;
	virtual String_t String() STD_VAL_THROW =0;
	virtual Bool_t Bool() STD_VAL_THROW =0;
};

////////////////////////////////////////////////////////////////////////////////
typedef std::vector<sens_value_t*> sens_list_t;
////////////////////////////////////////////////////////////////////////////////


#endif //ndef SENS_VALUE_T_H
