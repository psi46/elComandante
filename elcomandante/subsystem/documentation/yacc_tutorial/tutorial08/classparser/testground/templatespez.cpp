#include <iostream>
using namespace std;

// from: http://www.cplusplus.com/doc/tutorial/templates/
// "When we declare specializations for a template class, we must also define
// all its members, even those exactly equal to the generic template class,
// because there is no "inheritance" of members from the generic template to
// the specialization."
//
// 
// We just circumvent this by the following:
//


class base {
	int exp;
public:
	base(int Exp) throw() { exp=Exp; }
	virtual ~base() throw() {}

	virtual void func1() { cout << "\tstandard implementation func1" << endl; }
	virtual void func2() { cout << "\tstandard implementation func2" << endl; }
};

// derived template class:
template <class T>
class test : public base { 
public:	test(int Exp) : base(Exp) {};
};

// each specialisation is a copy of the template, plus extra overwritten functions:
template <>
class test<int> : public base {
public:
	test(int Exp) : base(Exp) {};
	virtual void func1() { cout << "\tint implementation func1" << endl; }
};

template <>
class test<double> : public base {
public:
	test(int Exp) : base(Exp) {};
	virtual void func2() { cout << "\tdouble implementation func2" << endl; }
};



int main(void) {
	base* obj[3];
	
	obj[0] = new test<int>(0);
	obj[1] = new test<double>(1);
	obj[2] = new test<long>(2);

	for (int i=0; i<3; ++i) {
		cout << "Object " << i << ":" << endl;
		obj[i]->func1();
		obj[i]->func2();
	}
	return 0;
}

/*
namespace test_templates {
	template <class X> class test;

	template <class T>
	void myfunc(test<T>* obj) { cout << "standard type" << endl; };

	template <>
	void myfunc<int>(test<int>* obj) { cout << "int" << obj->val << endl; }

	template <>
	void myfunc<double>(test<double>* obj) { cout << "double" << obj->val << endl; }
}; // end namespace test_templates

template <class T>
class test {
	T val;
public:
	void myfunc() { return test_templates::myfunc<T>(this); };
};
int main(void) {
	test<int> X;
	test<long> Y;
	X.myfunc();
	Y.myfunc();
	return 0;
}
*/
