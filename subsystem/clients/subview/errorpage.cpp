/*
 *	errorpage.cpp	12-Oct-2007
 *	Jochen Steinmann	<jochen.steinmann@rwth-aachen.de>
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "errorpage.h"

int errorpage::anzerror = 0;
int errorpage::lasterror = -1;
errorpage* errorpage::errorpagepointer = NULL;
char* errorpage::errorarray[MAXERRORS] = { NULL };

void errorpage::draw(){
	//pprintf(2,3, "ERROR");
	if ( anzerror < 1 ) return;		// no errors to print
	int startline = 3;
	int stopline = pageHeight() - 2;
	if ( stopline <= startline ) return;	// screen to small
	if (anzerror < stopline-startline ) stopline = startline + anzerror + 1;
	//int start = (lasterror - (stopline-startline) + 10*MAXERRORS) % MAXERRORS;	// first message on page
	for(int i = 0; i < (stopline-startline-1); i++){
		this->pprintf(startline+i, 2, errorarray[(lasterror -(stopline-startline-2)+ i+10*MAXERRORS) % MAXERRORS]);
	}

/*	for(int i=0; i< MAXERRORS; i++){
		this->pprintf(2+i, 2, errorpage::errorarray[i]);
	}
*/
}
errorpage::errorpage() : page_t() {
	this -> setTitle(" - ERROR - ");
	if (errorpagepointer != NULL)
		eerror("WARNING: errorpage instanciated multiple times!\n");
	errorpagepointer = this;
	for (int i=0; i<MAXERRORS; i++) { 	// allocate buffers
		errorarray[i] = (char*)malloc(MAXERRORLENGTH * sizeof(char));
		errorarray[i][0]=0;		// clear string;
	}
}
int errorpage::eerror(char* fmt, ...){ //FIXME segmentation fault
        va_list argptr;
	time_t now = time(NULL);
        char buffer[BUFFERSIZE];

        va_start(argptr, fmt);
        vsnprintf(buffer, BUFFERSIZE-1, fmt, argptr);
	va_end(argptr);

	anzerror++;
	lasterror++;
	lasterror %= MAXERRORS;

	strncpy(errorarray[lasterror], ctime(&now), MAXERRORLENGTH-2);
	errorarray[lasterror][24]=0;	// FIXME bad practice
	strncat(errorarray[lasterror], " - ", MAXERRORLENGTH-2-strlen(buffer));
	strncat(errorarray[lasterror], buffer, MAXERRORLENGTH-2-strlen(buffer));
	errorarray[lasterror][MAXERRORLENGTH-1] = '\0';	// secure buffer

	redraw();	// redraw the active page
	return 0;
}

