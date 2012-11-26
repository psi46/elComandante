/*
 * \file main.cpp
 * \author Dennis Terhorst
 * \date Thu Jul  2 20:35:43 CEST 2009
 */


#include "valuei_t.h"
#include "valued_t.h"
#include "valuesconst_t.h"
#include "action_context_t.h"
#include "packet_description_t.h"

#include <iostream>
using namespace std;

const char* packet= "123456789 hallo=42.14159265358979324, welt=33\n";

//stack<action_context_t> action_context;
//#define current_context	action_context.back()

#include <vector>
#include <string.h>	// strlen;
#include "subscript.h"

#define GLOBAL	"global"

typedef	vector<sensation_t*> sens_vec_t;
int main(void) {

	int parsefail = 0;
	do {	// here, in the main(), we want to restart until a successfull finish
		action_context_t ac(GLOBAL);
		cout << GLOBAL << "> " << flush;
		parsefail = subscript_parse(&ac);
		if ( parsefail ) {
			cout << "dropped out of global context" << endl;
			cout << "subscript exited with error! restarting.\n" << endl;
		}
	} while (parsefail);

	cout << "subscript successfully finished.\n" << endl;

	return 0;

	//////////////////////////////////////////////////////////////////////
	cout << "creating packet definition..." << endl;

	sens_value_t* sens;
	string search = "result";
	action_context_t ac("global");

	if ( ac.find_sens(search, sens) ) cout << search << " = " << sens->String() << endl;

	cout << "adding senses..." << endl;
	packet_description_t* pd = new packet_description_t("/test");
	pd->push_back(new sens_value_t("time", new valuei_t("s", value_t::NO_READ_UNIT) ) );
	pd->push_back(new sens_value_t("hallotag", new valuesconst_t("hallo=") ) );
	pd->push_back(new sens_value_t("result", new valued_t("ru", value_t::NO_READ_UNIT) ) );
	pd->push_back(new sens_value_t("welttag", new valuesconst_t(", welt=") ) );
	pd->push_back(new sens_value_t("welt", new valued_t("ru", value_t::NO_READ_UNIT) ) );
	pd->push_back(new sens_value_t("NL", new valuesconst_t("\n") ) );
	ac.add(pd);

	if ( ac.find_sens(search, sens) ) cout << search << " = " << sens->Double() << endl;

	cout << "scanning packet \"" << packet << "\":" << endl;
	pd->read_packet(packet);

	if ( ac.find_sens(search, sens) ) cout << search << " = " << sens->Double() << endl;
	ac.find_sens(search, sens);

	return 0;
/*	sens_vec_t sens;
	cout << "adding non-value sensations..." << endl;
	sens.push_back(new sens_sum_t(sens[0], sens[2]));
	sens.push_back(new sens_diff_t(sens[2], sens[4]));

	for ( sens_vec_t::iterator iter=sens.begin(); iter != sens.end(); ++iter) {
		cout << "sensation " << (*iter)->Name() << "\t= ";
		cout << "string:\"" << (*iter)->String() << "\"";
		cout << "\tint:" << (*iter)->Integer();
		cout << "\tdouble:" << (*iter)->Double() << endl;
	}

	for ( sens_vec_t::iterator iter=sens.begin(); iter != sens.end(); ++iter) {
		delete(*iter);
	}
	return 0;*/
}
