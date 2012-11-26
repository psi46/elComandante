
#include <vector>
class screen;

class element {
	screen* S;
public:
	element(screen* scr=NULL);
};

#include <vector>
typedef std::vector<element*> element_vector;
class screen : public element {
	element_vector ev;
public:
	screen():element(this) {
	}
	friend class element;
};

element::element(screen* scr) {
	S=scr;
	if ( scr->ev.size()>9) { scr=NULL; }
}

screen A;

#include <iostream>
using namespace std;
int main(void) {
	screen S;
	cout << "Hello World" << endl;
	return 0;
}
