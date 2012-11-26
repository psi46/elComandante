/*
 *	errorpage.h	12-Oct-2007
 *	Jochen Steinmann	<jochen.steinmann@rwth-aachen.de>
 */
#ifndef ERRORPAGE
#define ERRORPAGE

#define MAXERRORS 128
#define MAXERRORLENGTH 256

#include "page_t.h"

class errorpage : public page_t {
	
	private:
		static char* errorarray[MAXERRORS];
		static int anzerror;
		static int lasterror;
		static errorpage* errorpagepointer;
	public:
		errorpage();
		void draw();
		static int eerror(char* fmt, ...);
};

#endif
