
#include <iostream>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class value_t {
protected:
	bool init;
	int	num;
public:
	value_t(int Num) { cout << "value_t()" << endl; init=false; num=Num; }
	/*value(value& copy) {
		init = copy.init;
		num = copy.num;
	}*/
	virtual ~value_t() { cout << "~value_t()" << endl; }
	virtual value_t* copy()=0;

	void set(int Num) { init=true; num=Num; }
	virtual void print() { cout << "value.num=" << num << " (" << init << ")" << endl; }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <class T>
class valueT : public value_t {
public:
//	value(int Num) : value_t(Num) { cout << "value() INVALID ------------" << endl; }
//	virtual ~value() { cout << "~value() INVALID ---------------------"<< endl; }
	virtual void setval(T newval)=0;
};

////////////////////////////////////////////////////////////////////////////////

template <>
class valueT<double> : public value_t {
	double val;
public:
	valueT() : value_t(5) {
		cout << "value<double>()" << endl;
		val=0;
	}
	virtual ~valueT() { cout << "~value()" << endl; }
	virtual value_t* copy() { return new valueT<double>(*this); }

	void print() { cout << "value<double>.val=" << val; cout << "\tvalue.num=" << num << " (init " << (init?"true":"false") << ")" << endl; }
	virtual void setval(double v) { val=v; };
};

////////////////////////////////////////////////////////////////////////////////

#include <string>
template <>
class valueT<string> : public value_t {
	string val;
public:
	valueT() : value_t(5) {
		cout << "value<string>()" << endl;
		val="unknown";
	}
	virtual ~valueT() { cout << "~value<string>()" << endl; }
	virtual value_t* copy() { return new valueT<string>(*this); }

	void print() { cout << "value<string>.val=" << val; cout << "\tvalue.num=" << num << " (init " << (init?"true":"false") << ")" << endl; }
	virtual void setval(string v) { val=v; };
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <vector>
int main(void) {
	typedef vector<value_t*> vec_t;
	vec_t vec;

	value_t* X;	// we have *X

	cout << "create double object" << endl;
	valueT<double>* A = new valueT<double>();
	A->setval(42);
	vec.push_back(A->copy());
	cout << "delete double object" << endl;
	delete A;

	cout << "create string object" << endl;
	X = new valueT<string>();
	vec.push_back(X->copy());
	cout << "delete string object" << endl;
	delete X;

	cout << "vector:" << endl;
	for (vec_t::iterator i=vec.begin(); i!=vec.end(); ++i) {
		cout << "vec["<<i-vec.begin()<<"]: ";
		(*i)->print();
	}

	cout << "delete vec" << endl;
	for (vec_t::iterator i=vec.begin(); i!=vec.end(); ++i) {
		delete (*i);
	}
	cout << "delete done" << endl;
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

