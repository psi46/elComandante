/**
 * error.h	21-Oct-2007
 * Dennis Terhorst <dennis.terhorst@rwth-aachen.de>
 *
 * Wrapper header file for error ouput.
 */
#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#define FUNCTIONTRACKER fprintf(stderr, "calling %s:%d\n", __FILE__, __LINE__)
//#define FUNCTIONTRACKER {}

//#define USE_ERRORPAGE

#ifdef USE_ERRORPAGE
	#include "errorpage.h"
	#include <string.h>
	#include <errno.h>
	// printf should not be used with ncurses based errorpage
	#define eprintf(format, args...) errorpage::eerror(format , ##args)
	#define eperror(format, args...) errorpage::eerror(format , ##args)
	//#define eperror(format, args...) errorpage::eerror(format ": %s", ##args, strerror(errno))
#else
	#include <stdio.h>
	#include <errno.h>
	#include <string.h>
	#define eprintf(format, args...) printf(format , ##args)
	#define eperror(format, args...) printf(format ": %s\n", ##args, strerror(errno));
#endif

// FIND DEPRECATED
//#define printf(format, args...) printf("*** %s:%d printf is DEPRECATED, use eperror *** " format, __FILE__, __LINE__, ##args)
//#define perror(format, args...) perror("*** perror is DEPRECATED, use eperror *** " format)

// end ifdef ERROR_H
#endif





#ifndef __testfree_h__
#define __testfree_h__
#include <stdio.h>

#define free(x) printf("freeing "#x" %p at %s: %d\n", x, __FILE__, __LINE__); \
fflush(stdin);\
free(x);

#endif

