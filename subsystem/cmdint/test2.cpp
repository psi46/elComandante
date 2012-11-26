

#include <stdio.h>


///////////////////////////////////////////////////////////////////
#include "convert.h"
#include "cmdint.h"
///////////////////////////////////////////////////////////////////

class someclass { //someclass {
	int id;
public:
	someclass(int i)  { //someclass(int i) {
		id=i;
	}
	int print(float parm) {
		printf("object %d: testcall p1 float=%f\n", id, parm );
		return 0;
	}
};


#include <string.h>
#include "../error.h"
int main(void) {
	someclass asdf(0);

	//cmdint ci;			// FIXME: somehow the destruction
					//        of this causes segfaults.
	cmdint* ci = new cmdint();	// pointer version seems to be okay...
	
	ci->add1<someclass, float>("pint", &asdf, &someclass::print );
	ci->add1<someclass, float>("print", &asdf, &someclass::print );
	
	ci->print();
	
	char buffer[256];
	
	printf("\n*** INIT COMPLETE ***\n");

	while ( printf("> "), fflush(stdout), fgets(buffer, 256, stdin) ) {
		if ( strcmp("exit\n", buffer) == 0 ) break;
		if ( ci->execute(buffer) < 0 ) {
			eperror("\"%s\" execution failed", buffer);
		};
	};

	printf("delete ci\n");
	delete ci;
	printf("return main()\n");
	return 0;
};
