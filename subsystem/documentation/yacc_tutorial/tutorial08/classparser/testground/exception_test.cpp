#include "exceptions.h"

#include <iostream>

using namespace std;

void bla() throw(errno_exception<EACCES>) {
	throw errno_exception<EACCES>("you may not use this function");
	return;
}

int main(void) {

	try {
		bla();
	}
	catch (errno_exception<EACCES> &e) {
		cout << "e.what(): '"<< e.what() << "'" << endl;
	}
	cout << "end" << endl;
	return 0;
}
