
#include <pthread.h>
#include <iostream>
#include <vector>
#include <unistd.h>	// usleep
using namespace std;

typedef vector<int> testvec_t;
testvec_t testvec;

void* changefunc(void*) {
	int i=5;
	while (i--) {
		sleep(1);
		testvec.push_back(i);
	}
	cout << "pt::end" << endl;
	return 0;
}

int main(void) {

	pthread_t pt;
	
	cout << "hello world" << endl;

	(void) pthread_create(&pt, NULL, changefunc, NULL);

	int i=55;
	while (i--) {
		usleep(100000);
		cout << "testvec <";
		for (testvec_t::iterator i=testvec.begin(); i!=testvec.end(); ++i) {
			cout << (*i) << " ";
		}
		cout << ">" << endl;
	}

	(void) pthread_join(pt, NULL);
	return 0;
}
