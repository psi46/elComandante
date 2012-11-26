/**
 * \file packet_description_t.h
 * describes the different data fields in a packet
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
using namespace std;
#include "value_t.h"
#include "sensations.h"

typedef enum {PKT_DATA, PKT_SETDATA} packet_type_t;

class packet_description_t {
	typedef vector<sens_value_t*> sens_vec_t;


	string		name;
	packet_type_t	type;
	sens_vec_t	sens;
public:
	packet_description_t(string AboName, packet_type_t Type=PKT_DATA);
	virtual ~packet_description_t();

	void push_back(sens_value_t* sen);
	int read_packet(const char* text);

	bool get_sens(string request_name, sens_value_t*& theSens);
	bool operator==(packet_description_t& other);

	friend ostream& operator<<(ostream& os, packet_description_t* pd);
}; // end class packet_description_t




#endif // ndef PACKET_DESCRIPTION_T_H
