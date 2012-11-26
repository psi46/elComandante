


#include <iostream>
using namespace std;
/*class tree_t {
	int refcount;
	tree_t* parent;
public:
	tree_t(tree_t* Parent) {
	}
	virtual ~tree_t() {
	}
	int RefCount() const { return refcount; }
	
};
*/
class context_t {
	static int number;
	int thisnum;

	int refcount;
	context_t* parent;

public:
	context_t(context_t* Parent) {
		refcount=0;
		number++;
		thisnum=number;

		parent=Parent;
		if (parent!=NULL) {
			parent->refcount++;
			cout << "new context " << thisnum << " parent " << parent->thisnum << endl;
		} else {
			cout << "new context " << thisnum << endl;
		}
	}
	virtual ~context_t() {
		if (refcount > 0) throw(string("cannot delete referenced context_t"));
		if (parent!=NULL) {
			parent->refcount--;
		}
		//number--;
		cout << "del context " << thisnum << endl;
		thisnum=999;
	}
	friend ostream& operator<<(ostream &os, context_t &c);
	void printparents() const throw() {
		if (parent != NULL) parent->printparents();
		cout << " -> context" << thisnum << "(" << refcount << "refs)";
	}
	context_t* Parent() const throw() { return parent; }
	int RefCount() const { return refcount; }

};

ostream& operator<<(ostream &os, context_t &c) {
	os << " c" << c.thisnum << ": ";
	if (c.parent != NULL) c.parent->printparents();
	os << " -> context" << c.thisnum << "(" << c.refcount << "refs)";
}
int context_t::number = 0;

int main(void) {
	context_t* c1 = new context_t(NULL);		// main thread
	cout << " c1: "; c1->printparents(); cout << endl;
	context_t* c2 = new context_t(c1);		// main thread
	cout << " c2: "; c2->printparents(); cout << endl;
	context_t* c3 = new context_t(c2);		// main thread
	cout << " c3: "; c3->printparents(); cout << endl;
	context_t* c4 = new context_t(c1);		// t1
	cout << " c4: "; c4->printparents(); cout << endl;
	cout << "all:" << endl;
	cout << " c1: "; c1->printparents(); cout << endl;
	cout << " c2: "; c2->printparents(); cout << endl;
	cout << " c3: "; c3->printparents(); cout << endl;
	cout << " c4: "; c4->printparents(); cout << endl;

	try {
		delete c1;
	} catch (string &s) {
		cout << "catch: " << s << endl;
	}
	cout << " c2: "; c2->printparents(); cout << endl;
	// thread:
	cout << "say c3 has finished:" << endl;
	context_t* context = c3;
	// destructor:
	try {
		while (context) {
			context_t* newroot= context->Parent();
			if (context==NULL) break;
			delete (context);
			context = newroot;
		}
	} catch ( string &s) {
		cout << "not deleting " << *context << ": ref>0." << endl;
		// ignore
	}

	// end thread
	context_t* c5 = new context_t(c1);		// t2
	cout << " c5: "; c5->printparents(); cout << endl;

	cout << *c1 << "ref'd" << endl;
	cout << *c2 << "invalid" << endl;
	cout << *c3 << "deleted" << endl;
	cout << *c4 << "t1" << endl;
	cout << *c5 << "t2" << endl;

	return 0;
};
