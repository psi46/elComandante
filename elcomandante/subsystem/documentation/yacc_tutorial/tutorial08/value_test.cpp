
#include <iostream>
#include <string>
using namespace std;


/*
typedef enum {LINEAR, EXPO, STEP} extrapol_t;

template <extrapol_t E>
class sens_t {
	
};

*/


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


#include "valued_t.h"
#include "values_t.h"
#include "valuei_t.h"

const char* text="123456789: current =  4.432mA";
//const char* text=" 4.432mA";


int main(void) {
	cout << "hallo welt" << endl;

	const char* ptr = (char*)text;

	packet_description p("/test");
/*	valuei_t time("s", value_t::NO_READ_UNIT);
	values_t word(": current =");
	valued_t current("A");
	p.push_back( "time", time );
	p.push_back( "word", word );
	p.push_back( "current", current );*/
	p.push_back( "time", new valuei_t("s", value_t::NO_READ_UNIT | value_t::NO_WRITE_UNIT) );
	p.push_back( "word", new values_t(": current =") );
	p.push_back( "current", new valued_t("A") );
//	p.push_back( "time", static_cast<value_t>(valuei_t("s", valuei_t::NO_READ_UNIT)) );
//	p.push_back( "word", static_cast<value_t>(values_t(": current =")) );
//	p.push_back( "current", static_cast<value_t>(valued_t("A")) );

	cout << "read_packet return: " << p.read_packet(ptr) << endl;
//	cout << "rest: " << ptr << endl;
//	cout << "rest: " << ptr << " scan returned " << t.scan(ptr) << endl;
//	cout << "rest: " << ptr << " scan returned " << word.scan(ptr) << endl;
//	cout << "rest: " << ptr << " scan returned " << current.scan(ptr) << endl;
//	cout << "rest: " << ptr << endl;
//	cout << "time: " << t <<  " current = " << current << endl;
	return 0;
}
