/**
 * cmdint.h
 * lightweight command line interpreter classes for mapping
 * of strings to "object->function(parameter)" calls.
 *
 * There are three classes defined in this file:
 * 	cmdcall  -parent class for a generic commandname
 * 		  to function call mapping
 * 	cmdcallX -template class derived from cmdcall for
 * 		  a X parametric function call
 * 	cmdint	 -a class containing a list of cmdcalls
 * 		  which also does parsing and command
 * 		  selection
 *
 * Dennis Terhorst
 * 2008-06-29
 */
#ifndef CMDINT_H
#define CMDINT_H

#include <string.h>
#include "../error.h"
#include "convert.h"

#include <errno.h>
#define CMDINT_MAX_CMDS	64
#define CMDINT_DEBUG	0

#define CMDINT_MAX_NAMELEN	256
#define CMDINT_MAX_ARGS	10
#define CMDINT_MAX_CMDLINELEN	512

//////////////////////////////////////////////////////////////////////////////
class cmdcall {
protected:
	char name[CMDINT_MAX_NAMELEN];
	//char* name;
	//int CMDINT_MAX_NAMELEN;
protected:
	int parms;
public:
	cmdcall(const char* Name ) {
		if (strlen(Name) > CMDINT_MAX_NAMELEN) {
			eprintf("WARNING: Name to long, truncating!");
		}
		//CMDINT_MAX_NAMELEN = strlen(Name)+1;
		//name = (char*)calloc(1,CMDINT_MAX_NAMELEN);
		strncpy(name, Name, CMDINT_MAX_NAMELEN);

		if (CMDINT_DEBUG) printf("strncpy(%p,%p,%d)\n", name, Name, CMDINT_MAX_NAMELEN );
		if (CMDINT_DEBUG) printf("name = \"%s\"\n", name );
		if (CMDINT_DEBUG) printf("Name = \"%s\"\n", Name );
		name[CMDINT_MAX_NAMELEN-1]=0;	// ensure termination
		parms=0;
	}
	
	virtual ~cmdcall() {
		//free(name);
	};

	int Parameter() { return parms; }
	char* Name() {	// FIXME return const char*!
		return name;
	}

	bool operator==(const char* compareName) {
		
		if (CMDINT_DEBUG) printf("strncmp(%p,%p,%d)\n", this->name, compareName, CMDINT_MAX_NAMELEN );
		if (CMDINT_DEBUG) printf("name = \"%s\"\n", this->name );
		if (CMDINT_DEBUG) printf("compareName = \"%s\"\n", compareName );
		return (strncmp(name, compareName, CMDINT_MAX_NAMELEN) == 0);
	}
	bool operator!=(const char* Name) { return( !operator==(Name) ); }

	virtual int execute(int argc, char* argv[])=0;

};

//////////////////////////////////////////////////////////////////////////////  0 parameter cmdcall
template <class T>	// base object type, parameter type
class cmdcall0 : public cmdcall {
	T* obj;
	int (T::*func)();
public:
	cmdcall0(const char* Name, T* baseobj, int (T::*F)() ) : cmdcall(Name) {
		obj = baseobj;
		func = F;
		parms=0;
		if (CMDINT_DEBUG) FUNCTIONTRACKER;
	}
	~cmdcall0() {};

	int execute(int argc, char* argv[]) {
		if (CMDINT_DEBUG) FUNCTIONTRACKER;
		return (obj->*func)();
	}
};

//////////////////////////////////////////////////////////////////////////////  1 parameter cmdcall
template <class T, class P>	// base object type, parameter type
class cmdcall1 : public cmdcall {
	T* obj;
	int (T::*func)(P);
public:
	cmdcall1(const char* Name, T* baseobj, int (T::*F)(P) ) : cmdcall(Name) {
		obj = baseobj;
		func = F;
		parms=1;
		if (CMDINT_DEBUG) FUNCTIONTRACKER;
	}
	~cmdcall1() {};

	int execute(int argc, char* argv[]) {
		if (CMDINT_DEBUG) FUNCTIONTRACKER;
		P p1; convert(argv[1], &p1);
		if (CMDINT_DEBUG) FUNCTIONTRACKER;
		return (obj->*func)(p1);
	}
};

//////////////////////////////////////////////////////////////////////////////	2 parameter cmdcall
template <class T, class P, class Q>
class cmdcall2 : public cmdcall {
	T* obj;
	int (T::*func)(P, Q);
public:
	cmdcall2(const char* Name, T* baseobj, int (T::*F)(P, Q) ) : cmdcall(Name) {
		obj = baseobj;
		func = F;
		parms=2;
		if (CMDINT_DEBUG) FUNCTIONTRACKER;
	}
	~cmdcall2() {};

	int execute(int argc, char* argv[]) {
		if (CMDINT_DEBUG) FUNCTIONTRACKER;
		P p1; convert(argv[1], &p1);
		Q p2; convert(argv[2], &p2);
		if (CMDINT_DEBUG) FUNCTIONTRACKER;
		return (obj->*func)(p1, p2);
	}
};

//////////////////////////////////////////////////////////////////////////////	3 parameter cmdcall
template <class T, class P, class Q, class R>
class cmdcall3 : public cmdcall {
	T* obj;
	int (T::*func)(P, Q, R);
public:
	cmdcall3(const char* Name, T* baseobj, int (T::*F)(P, Q, R) ) : cmdcall(Name) {
		obj = baseobj;
		func = F;
		parms=3;
		if (CMDINT_DEBUG) FUNCTIONTRACKER;
	}
	~cmdcall3() {};

	int execute(int argc, char* argv[]) {
		if (CMDINT_DEBUG) FUNCTIONTRACKER;
		P p1; convert(argv[1], &p1);
		Q p2; convert(argv[2], &p2);
		R p3; convert(argv[3], &p3);
		if (CMDINT_DEBUG) FUNCTIONTRACKER;
		return (obj->*func)(p1, p2, p3);
	}
};


//////////////////////////////////////////////////////////////////////////////
//
//   Command Interpreter
//
//////////////////////////////////////////////////////////////////////////////
class cmdint {
	int noc;
	cmdcall* cmds[CMDINT_MAX_CMDS];
public:

	cmdint() {
		noc=0;
	}
	~cmdint() {
		if (CMDINT_DEBUG) printf("noc = %d\n", noc);
		for (int i=0; i<noc; i++) {
			if (CMDINT_DEBUG) printf("delete %p [%d]\n", cmds[i], i);
			delete(cmds[i]);
		}
	}

	template <class T>						// 0 parameter function
	int add0(const char* Name, T* baseobj, int (T::*F)() ) {
		if (noc>=CMDINT_MAX_CMDS) return -1;
		cmds[noc++] = new cmdcall0<T>(Name, baseobj, F);
		if (CMDINT_DEBUG) printf("added  %p [%d]\n", cmds[noc-1], noc-1);
		return 0;
	}; // end add()
	
	template <class T, class P>					// 1 parameter function
	int add1(const char* Name, T* baseobj, int (T::*F)(P) ) {
		if (noc>=CMDINT_MAX_CMDS) return -1;
		cmds[noc++] = new cmdcall1<T,P>(Name, baseobj, F);
		if (CMDINT_DEBUG) printf("added  %p [%d]\n", cmds[noc-1], noc-1);
		return 0;
	}; // end add()
	
	template <class T, class P, class Q>				// 2 parameter function
	int add2(const char* Name, T* baseobj, int (T::*F)(P,Q) ) {
		if (noc>=CMDINT_MAX_CMDS) return -1;
		cmds[noc++] = new cmdcall2<T,P,Q>(Name, baseobj, F);
		if (CMDINT_DEBUG) printf("added  %p [%d]\n", cmds[noc-1], noc-1);
		return 0;
	}; // end add()
	
	template <class T, class P, class Q, class R>			// 3 parameter function
	int add3(const char* Name, T* baseobj, int (T::*F)(P,Q,R) ) {
		if (noc>=CMDINT_MAX_CMDS) return -1;
		cmds[noc] = new cmdcall3<T,P,Q,R>(Name, baseobj, F);
		if (CMDINT_DEBUG) printf("added  %p [%d]\n", cmds[noc], noc);
		noc++;
		if (CMDINT_DEBUG) print();
		return 0;
	}; // end add()

	int print() {
		char* t;
		printf("\n*** cmds[] pointers ***\n");
		for (int i=0; i<noc; i++) {
			printf("\tcmds[%d] @ %p\n", i, cmds[i]);
		}
		printf("\n*** name pointers ***\n");
		for (int i=0; i<noc; i++) {
			t= cmds[i]->Name();
			printf("\t%p cmds[%d].Name()\n", t,i);
			printf("\t\"%s\" cmds[%d].Name()\n", t,i);
		}
		return 0;
	}

	int execute(char* cmdline) {
		if (CMDINT_DEBUG) print();
		int argc=CMDINT_MAX_ARGS;
		char* argv[CMDINT_MAX_ARGS];

		parse(cmdline, argc, argv);
		if (CMDINT_DEBUG) for (int i=0; i<argc; i++) {
			printf("ci.exec: argv[%d]=\"%s\"\n", i, argv[i]);
		}
		if (argc < 1) return 0;	// discard empty lines
		if (CMDINT_DEBUG) printf("searching in %d commands...\n", noc);
		for (int i=0; i<noc; i++) {
			if (CMDINT_DEBUG) printf("cmds[%d]@%p ?= %s\n", i, cmds[i], argv[0] );
			if (*(cmds[i]) != argv[0] ) continue;
			if (CMDINT_DEBUG) printf("cmds[%d] == %s !\n", i, argv[0] );
			
			if (argc-1 != cmds[i]->Parameter()) {
				eprintf("wrong number(%d) of parameters(%d) to \"%s\" command!\n",
					argc-1, cmds[i]->Parameter(), cmds[i]->Name());
				errno = EINVAL;
				return -1;
			}
			if (CMDINT_DEBUG) printf("correct number of parameters\nexec\n");
			return cmds[i]->execute(argc, argv);
		}
		errno = ENOSYS;
		return -1;
	}; // end execute()

	// parse text into tokens argv
	int parse(const char* cmdline, int& argc, char* argv[]) {
		if (cmdline == NULL) return -1;
		//int maxargs=argc; FIXME
		if (argc < 1) return -1;
		int textlen = strlen(cmdline);
		char text[CMDINT_MAX_CMDLINELEN];
		strncpy(text, cmdline, CMDINT_MAX_CMDLINELEN);
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
	}; // end parse()
};

#endif //ndef CMDINT_H
