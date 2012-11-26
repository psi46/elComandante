#include <iostream>
using namespace std;

class myex {
public:
	int i;
	myex(int X) { i=X; }
	virtual ~myex() {};
};

void funcb() throw(myex) {
	throw(myex(4));
}

void funca() throw() {
	funcb();
}


int main(void) {

	try {
		funcb();
	}
	catch (myex& e) {
		cout << "catched myex " << e.i << endl;
	}
	


	return 0;
}
