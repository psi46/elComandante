#include <iostream>
using namespace std;


class base {
public:
	virtual void func() { cout << "base.func" << endl; }
};

class mid : public base {
public:
};

class bot : public mid {
public:
	virtual void func() { cout << "bot.func" << endl; }
};

int main(void) {
	base* p = new bot;
	p->func();
	return 0;
}
