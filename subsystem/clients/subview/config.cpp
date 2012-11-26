/*	config.cpp 	30.12.2007
 *      Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */

#include "config.h"
#include "error.h"

/**
 * Konstruktor opens the configuration, which is given by the filename
 */
config::config(char* filename){
	myfilename = filename;
	fp = fopen(myfilename, "r");
	group = NULL;
	conf_init(&conf);
	conf_read(&conf, fp);
	fclose(fp);
}
/**
 * Destruktor
 */
config::~config(){
	conf_destroy(&conf);
}

/**
 * Set the data which is in the selected group and value
 */
void config::setDouble(char* ch_group, const char* value, double data){
	group=conf_get_group(&conf,ch_group);
	char buffer[16];
	sprintf(buffer,"%lf", data);
	group_set_value(group, value, buffer);
	fp = fopen(myfilename, "w");
	conf_write(&conf, fp);
	eprintf("Configuration saved");
	fclose(fp);
}

/**
 * \return the Integer-Value, which in the selected group and value
 */
int config::getInt(char* ch_group, const char* value){
	int returnval;
	group=conf_get_group(&conf,ch_group);
    	if(group!=NULL){
        	if (1!=sscanf(group_get_value(group, value), "%d", &returnval)) {
           		 return 0;
        	}else{
			return returnval;
		}
    	}else{
		return 0;
	}
}
/**
 * \return the double-Value, which in the selected group and value
 */
double config::getDouble(char* ch_group, const char* value){
	float returnval;
	group=conf_get_group(&conf,ch_group);
    	if(group!=NULL){
        	if (1!=sscanf(group_get_value(group, value), "%f", &returnval)) { 
           		 return 0.0;
        	}else{
			return returnval;
		}
    	}else{
		return 0.0;
	}
}
/**
 * \return the const char-Value, which in the selected group and value
 */
const char* config::getChar(char* ch_group, const char* value){
	group=conf_get_group(&conf,ch_group);
    	if(group!=NULL){
		return group_get_value(group, value);
    	}else{
		return NULL;
	}
}
/**
 *	ReadIn Mixture from Config file
 */
/*
mixture config::getMixture(char* ch_group){
	mixture mix;
	group=conf_get_group(&conf,ch_group);
	double help = 0.0;
    	if(group!=NULL){
		for(int i=0; i<MAX_GAS; i++){
	        	if (1!=sscanf(group_get_value(group, mixture::gas_names[i]), "%lf", &help)) { 
           			mix[i] = 0.0;	
        		}else{
				mix[i] = help;
			}
		}
	}
	return mix;
}
*/
