/**
 * \file sub_config.h
 * \author Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */
#ifndef SUB_CONFIG_H
#define SUB_CONFIG_H

#include "../../error.h"
#include "../../sclient.h"
#include "../../packet_t.h"
#include "../../error.h"

/**
 * Wrapper to the config program
 */
class sub_config {
	public:
		sub_config(char* config_abo);
		~sub_config();

		int getInt(const char* ch_group, const char* value);

//		void setDouble(const char* ch_group, const char* value, double value);
		double getDouble(const char* ch_group, const char* value);
		const char* getChar(const char* ch_group, const char* value);

	private:	
		char* myAbo;
		packet_t packet;
		sclient me;

};

#endif
