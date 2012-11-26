/**
 * \file packet_description_t.h
 * describes the different data fields in a packet.
 *
 * \author Dennis Terhorst
 * \date Wed Jul  1 17:36:39 CEST 2009
 */
#ifndef PACKET_DESCRIPTION_T_H
#define PACKET_DESCRIPTION_T_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
//#include "value_t.h"
#include "exceptions.h"
#include "sens_value_t.h"

//typedef enum {PKT_DATA, PKT_SETDATA} packet_type_t;
#define USE_PACKET_TYPE_ENUM
#include <subsystem/packet_t.h>

extern const char* packet_type_name[];

/**
 * \brief description of the packet structure
 *
 * This class stores a definition in terms of sens_value_t pointers, which will
 * be sequentially read/parsed from a packet, receiving their values.
 */
class packet_description_t { typedef std::vector<sens_value_t*> sens_vec_t;

	std::string	name;
	packet_type_t	type;
	sens_vec_t	sens;
	bool		hasdata;
public:
	packet_description_t(std::string name="unnamed", packet_type_t Type=PKT_DATA);
	packet_description_t(const packet_description_t& copy);
	virtual ~packet_description_t();

	std::string Name() const throw();
	packet_type_t Type() const throw();

	void push_back(sens_value_t* sen);
	int read_packet(const char* text);
	bool hasData() const throw();

	bool get_sens(std::string request_name, sens_value_t*& theSens);
	bool operator==(const packet_description_t& other) const throw();
	bool operator==(const std::string& Name) const throw();

	sens_value_t* lookup(std::string name) throw(errno_exception<ENOMSG>);

	friend std::ostream& operator<<(std::ostream& os, packet_description_t* pd);
	friend std::ostream& operator<<(std::ostream& os, packet_description_t& pd);
}; // end class packet_description_t


#endif // ndef PACKET_DESCRIPTION_T_H
