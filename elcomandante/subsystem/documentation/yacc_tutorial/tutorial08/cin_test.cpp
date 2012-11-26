#include <iostream>
using namespace std;

int main(void) {
	char c;
	do {
		cin >> c;
		cout << c << flush;
	} while (c != 'x');
	return 0;
}
