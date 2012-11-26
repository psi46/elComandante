/**
 * \file error.h
 * \author Dennis Terhorst <dennis.terhorst@rwth-aachen.de>
 * \date 21-Oct-2007
 *
 * \brief Wrapper header file for error ouput.
 *
 * Inclusion of this header file defines the eprintf and eperror macros,
 * which can be used for a centralized logging interface. Redefinig these
 * to whatever logging function should be used makes it possible to change
 * the path of debug output at compile time.
 */
#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

/**
 * \brief set a program marker
 *
 * calling this macro in a program or subroutine causes the position to be
 * printed to stderr. You will see a line like <TT>calling foo.cpp:42</TT>
 * on stderr which indicates that the program execution has passed line 42 in
 * file foo.cpp.
 *
 * If all debuging output shall be supressed, this can be defined as empty
 * call (<TT>{}</TT>), effectively removing all output.
 */
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
	#ifndef eprintf
		#define eprintf(format, args...) fprintf(stderr, format , ##args)
	#endif
	#ifdef _WIN32	// WINDOWS
		#warning Achtung WINDOWS
		//#define eperror(format, args...) fprintf(stderr, format ": %s\n", ##args, WSAGetLastError())
		#ifndef eperror
			#define eperror(format, args...) fprintf(stderr, format , ##args)
		#endif
	#else			// LINUX
		#ifndef eperror
			#define eperror(format, args...) fprintf(stderr, format ": %s\n", ##args, strerror(errno))
		#endif
	#endif
#endif

/**
 * \def eprintf
 * \brief print a message to the debug output channel
 *
 * This macro can be used just like a normal printf() for debug output. It will write the output using a function defined here (default is fprintf(stderr, ...) )
 */

/**
 * \def eperror
 * \brief print a message to the debug output channel including error description
 *
 * This macro can be used just like a normal printf() for debug output. It
 * will write the output using a function defined here (default is
 * fprintf(stderr, ...) ). Additionally to eprintf() this function will
 * append a string corresponding to the current \c errno. This is just like
 * the perror library funcion, but eperror also takes variable argument
 * lists like printf.
 */

// FIND DEPRECATED
//#define printf(format, args...) printf("*** %s:%d printf is DEPRECATED, use eperror *** " format, __FILE__, __LINE__, ##args)
//#define perror(format, args...) perror("*** perror is DEPRECATED, use eperror *** " format)


// end ifdef ERROR_H
#endif
