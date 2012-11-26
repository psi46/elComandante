//
// Test program for slist and obj
//

#include "slist.h"

#include <iostream>
using namespace std;

//
//  DEBUGGING TESTS BELOW THIS LINE
//
#define TEST_SLISTTEST_H
#ifdef  TEST_SLISTTEST_H

#include "object.h"

#define R int
//#define PRINT printf("X=%d, Y=%d, *Z=%d\n", (int)X, (int)Y, (int)*Z)
#define PRINT cout << "list=" << list << endl;

int main() {
	printf("Init...\n");
	slist<R> list;
	R X(13), Y(2), W(6), S(9);
	R *Z = new R(4);
	PRINT;
	printf("\nRun...\n");
/*	if ( X == Y ) {	// test obj operator== and operator!=
		printf("X == Y\n");
	} else {
		printf("X != Y\n");
	}
	X = Y;
	PRINT;
	if ( X != Y ) {
		printf("X != Y\n");
	} else {
		printf("X == Y\n");
	}*/

	list += X;	// copy into list;
	list += X;
	PRINT;
	list += Y;
	list += X;
	list += Y;

	cout << "list + W + S" << endl;
	list = list + W + S;
	PRINT;
	
	if ( *Z == (X+Y) ) {
		printf("*Z == X+Y\n");
	} else {
		printf("*Z != X+Y\n");
	}
	*Z += X;
	PRINT;

	list -= X;
	PRINT;
	
	list = list - Y - X;
	PRINT;

	// operator int()
	//int A = X;
	//printf(" X = %d\n", A);
//	printf(" X = %d\n", X);	// ==> warning: cannot pass objects of non-POD type `class obj'
				//		through `...'; call will abort at runtime
	printf("end of test run.\n");
	
}
#endif
