
#include <iostream>
using namespace std;

int main(void) {
	enum flags_t { NO=0, A=1, B=2, AB=3 };
	
	enum flags_t X=B;
	if ( X & A ) cout << "contains A" << endl;
	if ( X & B ) cout << "contains B" << endl;
	return 0;
}
