#include <iostream>
using namespace std;

class X {
public:
	void operator_dot(int x) { cout << "op . " << x << endl; }
	void operator+(int x)    { cout << "op + " << x << endl; }
};

int main(void) {
	X x;
	double Y = 4.5;

	x . operator_dot(4);
	x . operator+(4);
	cout << "++double: " << ++Y << endl;
	return 0;
}
