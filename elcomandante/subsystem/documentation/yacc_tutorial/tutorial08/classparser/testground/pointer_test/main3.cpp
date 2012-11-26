
#include <iostream>
#include <string>
using namespace std;

class super {
public:
	virtual string name() { return "super"; }
	virtual char type()=0;
};

template <char X>
class sub : public super {
public:
	virtual string name() { return "sub"+X; }
	virtual char type() { return X; };
};


template <char X>
void print(sub<X> *obj) {
	cout << "print<" << X << ">: " << obj->name() << endl;
}

int main(void) {
	super* arr[3];
	arr[0] = new sub<'A'>();
	arr[1] = new sub<'B'>();
	print(arr[0]);
	print(arr[1]);
	return 0;
}
