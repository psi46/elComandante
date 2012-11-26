
#include <iostream>
#include <string>
using namespace std;

void skip_ws(char*& ptr) throw() {
	while (*ptr == ' ' || *ptr == '\t' ) { ptr++; }
} // end skip_ws

int main(void) {
		const char* DATA=" measurement done\n";
		string value    = "measurement";

		char* ptr= const_cast<char*>(DATA);
		char* eptr=ptr;
		skip_ws(eptr);
		cout << "valueconstT<String_t>::scan_value() comparing input \"" << eptr << "\" to own value \"" << value << "\"" << endl;
		int ret;
		if ( (ret=value.compare(0, value.size(), eptr, value.size())) == 0 ) { eptr+=value.size(); ptr=eptr; cout << "return 0: rest=\"" << ptr << "\"" << endl; return 0; }
		cout << "valueconstT<String_t>::scan_value() input did not compare. ret=" << ret << "." << endl;
		cout << "return -1" << endl;
		return -1;
}
