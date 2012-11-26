
#include <iostream>
#include <string>
#include <stdexcept>
using namespace std;

class super {
public:
	typedef enum {subA, subB} subtype_t;

	virtual string name() { return "super"; }
	virtual subtype_t type()=0;	// every subtype has to know its type

	virtual super* operator+(super* other) { throw(runtime_error(operatorerror("+", other))); }
	virtual super* operator-(super* other) { throw(runtime_error(operatorerror("-", other))); }

private:
	string operatorerror(string op, super* other) {
		return string("operation not implemented: '") + typeid(*this).name()+ " "+ op +" "+ typeid(*other).name()+"'";
	}
};

class subA : public super {
public:
	virtual string name() { return "subA"; }
	virtual subtype_t type() { return super::subA; };
	virtual super* operator+(super* other) {
		switch (other->type()) {
		case super::subA: cout << "A+A" << endl; return this;
		case super::subB: cout << "A+B" << endl; return other;
		}
		throw(runtime_error("operation not implemented"));
	}
};

class subB : public super {
public:
	virtual string name() { return "subB"; }
	virtual subtype_t type() { return super::subB; };
	virtual super* operator+(super* other) {
		switch (other->type()) {
		case super::subA: cout << "B+A" << endl; return this;
		case super::subB: cout << "B+B" << endl; return other;
		}
		throw(runtime_error("operation not implemented"));
	}
};

int main(void) {
	super* arr[3];
	arr[0] = new subA();
	arr[1] = new subB();
	super* sum = *arr[1] + arr[0];
	super* diff = *arr[1] - arr[0];
	return 0;
}
