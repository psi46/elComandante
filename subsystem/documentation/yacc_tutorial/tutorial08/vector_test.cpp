#include <vector>
#include <iostream>
using namespace std;

class asdf {
	int x;
public:
	asdf(int X) { cout<<"constructor asdf("<<X<<")"<<endl; x=X; }
	virtual ~asdf() {};
	friend ostream& operator<<(ostream& os, asdf X);
};

ostream& operator<<(ostream& os, asdf X) {
	return os << X.x;
}

ostream& operator<<(ostream& os, vector<asdf> X) {
	os << "(";
	for (vector<asdf>::iterator iter=X.begin(); iter != X.end(); ++iter) {
		os << *iter <<" ";
	}
	return os << ")";
}

int main(void) {
	asdf A(3);
	asdf B(4);
	asdf C(5);

	vector<asdf> V;
	vector<asdf> V2;

	V.push_back(asdf(6));
	V.push_back(B);
	V.push_back(C);
	V2=V;
	V.clear();
	cout << "Hello " << A << endl;
	cout << "Vector: " << V << endl;
	cout << "Vector: " << V2 << endl;
	return 0;
}
