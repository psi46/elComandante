/*
 *	config.h 	30.12.2007
 *	Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */
#ifndef CONFIG_H
#define CONFIG_H

#include "conf.h"

/**
 * Wrapper to the config program
 */
class config {
	public:
		config(char* filename);
		~config();

		void setDouble(char* ch_group, const char* value, double value);

		int getInt(char* ch_group, const char* value);
		double getDouble(char* ch_group, const char* value);
		const char* getChar(char* ch_group, const char* value);

	private:
		char* myfilename;
		conf_t conf;
		FILE* fp;
		group_t* group;

};

#endif
