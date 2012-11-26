/*
 *	config.h 	30.12.2007
 *	Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */
#ifndef CONFIG_H
#define CONFIG_H

#include "conf.h"
#include "../../error.h"
/**
 * Wrapper to the config program
 */
class config {
	public:
		config(char* filename);
		~config();

		void setDouble(const char* ch_group, const char* value, double value);

		int getInt(const char* ch_group, const char* value);
		double getDouble(const char* ch_group, const char* value);
		const char* getChar(const char* ch_group, const char* value);

	private:
		char* myfilename;
		conf_t conf;
		FILE* fp;
		group_t* group;

};

#endif
