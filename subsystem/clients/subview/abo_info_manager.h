/**
 * \file abo_info_manager.h
 * \author Dennis Terhorst
 * \date Sun Sep 13 19:00:56 CEST 2009
 */
#ifndef ABO_INFO_MANAGER_H
#define ABO_INFO_MANAGER_H

#include <map>
#include <string>
#include "abo_info.h"
class abo_info_manager {
	typedef std::map<std::string, abo_info> infomap_t;
	infomap_t infos;
public:
	abo_info_manager();
	virtual ~abo_info_manager() {};
};



#endif //ndef ABO_INFO_MANAGER_H
