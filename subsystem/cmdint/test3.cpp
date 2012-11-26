#include <stdio.h>
#include <string.h>

class super {
protected:
	char name[256];
public:
	super(char* N) {
		printf("%p super.name\n", name);
		printf("%p super.super().N\n", N);
		strncpy(name, N, 256);
	}
};


class son : public super {
	int x;
public:
	son(char* N, int X) : super(N) {
		printf("%p son.name\n", name);
		printf("%p son.son().N\n", N);
		printf("%s son.name\n", name);
		printf("%s son.son().N\n", N);
		x=X;
	}
	
};

template <class T>
class doughter : public super {
	T x;
public:
	doughter(char* N, T X) : super(N) {
		printf("%p doughter.name\n", name);
		printf("%p doughter.son().N\n", N);
		printf("%s doughter.name\n", name);
		printf("%s doughter.son().N\n", N);
		x=X;
	}
	
};
int main(void) {

	super* asdf = new son("hallo", 3);
	super* sdfg = new doughter<int>("hallo", 4);

	printf("%p asdf\n", asdf);
	printf("%p sdfg\n", sdfg);

	return 0;
}
