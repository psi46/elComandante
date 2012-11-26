/*
 * \file command_t.h
 * \author Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 * \date 13 Mar 2008
 */
#ifndef COMMAND_T_H
#define COMMAND_T_H

//#include "clientinfo.h"	ADDINFO
//class subserver;		TARGET

#define VOIDFUNCPTR	void (*)(void)

// TARGET class is the class controlled with commands
// ADDINFO is the additional information supplied to the commands, other than argv[]
//	int (TARGET::*func)(ADDINFO* client, int argc, char* argv[]);

/**
 * \brief store a command name together with a function pointer
 *
 * This class stores a command name together with a function pointer and
 * additionally provides a static parse() function. This class is used by
 * subserver to identify and execute server commands (PKT_MANAGEMENT).
 *
 * Generally this class has some unresolvable disadvantages and is therefore
 * marked as deprecated. The parse() function has been copied to cmdint.
 *
 * \deprecated Use the cmdint classes instead.
 * \sa cmdint
 */
template <class FUNCPTRTYPE>
class command_t {
public:
	char* name;
	FUNCPTRTYPE func;
	//int (TARGET::*func)(ADDINFO* client, int argc, char* argv[]);
public:
	command_t(const command_t<FUNCPTRTYPE>& copy) {  // copy constructor
		*this = copy;
	}

	command_t(char* Name, FUNCPTRTYPE Func )
	{
		name = Name;
		func = Func;
	}

	command_t<FUNCPTRTYPE>& operator=(const command_t<FUNCPTRTYPE>& other) {      // operator =
		name = other.name;
		func = other.func;
		return *this;
	}

	bool operator==(const command_t<FUNCPTRTYPE>& other) const {   // operator ==
		return (strcmp(name, other.name)==0);
	}


	/**
	 * \brief 	 parse text into tokens argv[]
	 *
	 * The string pointed to by \a text is parsed into the \a argc,\a argv[]
	 * arrays by replacing each space or tab with the string termination
	 * '\\0' character. The pointers \a argv[] are set to the beginning of
	 * each token, \a argc is the total count of tokens.
	 *
	 * In a second step all empty tokens (caused by multiple spaces or tabs)
	 * are removed from the list.
	 *
	 * \warning Length of argv is not checked! It must be large enough to hold
	 *	    all tokens.
	 * \todo Make parse() aware of argv[] array length.
	 */
	static int parse(char* text, int textlen, int& argc, char* argv[]) {
		if (text == NULL) return -1;	// FIXME: argc is not checked
		argc = 0;
		// parse mgm commands
		argv[argc++]=text;
		for (int i=0; i<textlen; i++) {	// arg splitting
			if (argv[0][i]==' ' || argv[0][i] == '\t') {
				argv[0][i]='\0';
				argv[argc++] = &(argv[0][i+1]);
			}
			if (argv[0][i]=='\n') {
				argv[0][i]='\0';
				break;
			}
		}
		for (int i=0; i<argc; i++) {			// empty arg suppression
			if (strlen(argv[i]) == 0) {
				for (int j=i; j<argc; j++)
					argv[j] = argv[j+1];
				argc--;
				i--;
			}
		}
		return 0;
	}
};

#endif
