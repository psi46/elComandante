/**
 * \file config_test.cpp
 * \author Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */

#include <stdio.h>
#include <string>	// memcpy

#include "sub_config.h"

#define CONFIGURE_ABO "/configure"

int main(int argc, char *argv[]) {

	sub_config* myconf = new sub_config(CONFIGURE_ABO);
	printf("received %d\n",myconf -> getInt("chamber","testint"));
	printf("received %lf\n",myconf -> getDouble("chamber","test2"));
	printf("received %s\n",myconf -> getChar("chamber","name"));

	return 0;
}



