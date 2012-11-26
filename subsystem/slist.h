/**
 * \file slist.h
 *
 * \brief array with unique elements
 *
 * List which holds at most one element of each kind. Objects may
 * be of any kind (template \<\>), but must implement basic
 * operators: ==, =, << and constructors.
 *
 * \author Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 * \date Fri Feb 22 2008
 *
 */
#ifndef SLIST_H
#define SLIST_H

// To use output streams for whole lists
//#define SLIST_WITH_STREAMS

// how many elements to realloc, if current allocation
// can not hold a new element anymore
#define SLIST_REALLOC_SIZE	1
#include "error.h"
//#include <errno.h>
#include <stdlib.h>	// malloc, free
#include <string.h>	// memcpy


#ifdef SLIST_WITH_STREAMS	
#include <iostream>
using namespace std;
#endif


template <class T>
class slist {
private: // data members
	T*	list;		// array to hold element data
	int	nelem;		// number of valid elements in list[]
	int	alloced;	// number of SLIST_REALLOC_SIZE blocks allocated

public: // *structors
	slist() {
		list = NULL;
		nelem=0;
		alloced=0;
	}

	slist(const slist<T>& copy) : slist() {
		*this = copy;
	}

	~slist() {
		free(list);
	}

private: // functions
	int grow() {
		if ( nelem < alloced * SLIST_REALLOC_SIZE ) return -1;
		errno=0;
		T* listsave=list;
		list = (T*)realloc(list, (alloced+1)*SLIST_REALLOC_SIZE * sizeof(T));
		if ( list == NULL || errno != 0 ) {
			eperror("slist WARNING: could not realloc bigger list");
			list=listsave;
			return 0;
		}
		alloced++;
		return 1;
	}

	int shrink() {
		if ( nelem >= (alloced-1) * SLIST_REALLOC_SIZE ) return -1;
		errno=0;
		T* listsave=list;
		list = (T*)realloc(list, (alloced-1)*SLIST_REALLOC_SIZE * sizeof(T));
		if ( list == NULL || errno != 0 ) {
			eperror("WARNING: could not realloc smaller list");
			list=listsave;
			return 0;
		}
		alloced--;
		return 1;
	}

public: // functions
	int add(const T& obj) {
		for ( int i=0; i<nelem; i++) {
			if ( obj == list[i] ) { // compare objects, not pointers!
				return 0;
			}
		}
		//printf("slist add() object copy to list\n");
		grow();
		list[nelem++] = obj; // copy
		return 1;
	}

	int remove(const T& obj) {
		for ( int i=0; i<nelem; i++) {
			if ( obj == list[i] ) { // compare objects, not pointers!
				//cout << "slist remove() found " << obj << " in list" << endl;
				list[i] = list[--nelem]; // copy last obj down, delete obj
				shrink();
				return 1;
			}
		}
		//cout << "slist remove() could not find " << obj << " in list" << endl;
		return 0;
	}
	
	void clear() {
		while (remove(list[0]));
		return;
	}

	T* get(const T& sample) {
		for ( int i=0; i<nelem; i++) {
			if ( sample == list[i] ) { // compare objects, not pointers!
				return &(list[i]);
			}
		}
		return NULL;
	}

	int length() const {
		return nelem;
	}

public: // operators
	slist<T>& operator=(const slist<T>& copy) {
		if ( this == &copy ) return *this;
		
		int listsize = (copy.alloced)*SLIST_REALLOC_SIZE * sizeof(T);

		T* savelist = list;
		list = (T*)realloc(list, listsize);
		if ( list != NULL) {
			alloced=copy.alloced;
			memcpy(list, copy.list, listsize);
			nelem=copy.nelem;
			
		} else {
			list = savelist;
			eperror("ERROR: could not copy slist due to memory reallocation error!");
		}
	}
	T& operator[](int n) const {
		if ( n<0 || n>=nelem ) {
			eprintf("WARNING: out of bound slist index!");
			return list[0];	// FIXME this should return an error value!
		}
		return list[n];
	}

/*	operator int() {	// probably better not to do this
		return nelem;
	}*/

	slist<T>& operator+(const slist<T>& lst) {	// add other slist
		//cout << "slist + " << lst << endl;
		return slist<T>(lst)+=*this;
	}

	slist<T>& operator+=(const slist<T>& lst) {	// add other slist
		//cout << "slist + " << lst << endl;
		for (int i=0; i<lst.length(); i++)
			operator+(lst[i]);
		return *this;
	}
	slist<T>& operator+(const T& obj) {	// add element FIXME: const
		//return ((slist<T>(*this))+=obj);
		
		// NOTE: following lines do not copy slist!
		//       listA = listB + T   will change listB
		add(obj);
		return *this;
	}
	slist<T>& operator+=(const T& obj) {	// add element
		//cout << "slist += " << obj << endl;
		add(obj);
		return *this;
	}
	slist<T>& operator-(const T& obj)  // FIXME: should be const
	//{ return ((slist<T>(*this))-=obj); }
	{ remove(obj); return *this; } // FIXME: will NOT work as expected!
	
	slist<T>& operator-=(const T& obj) { remove(obj); return *this; }

#ifdef SLIST_WITH_STREAMS	
	friend ostream& operator <<(ostream& os, const slist<T>& list) {
		bool komma=false;
		os << "{";
		for (int i=0; i<list.nelem; i++) {
			if (komma) os << ",";
			os << list[i];
			komma=true;
		}
		return os << "} " << list.nelem << " elements in " << list.alloced * SLIST_REALLOC_SIZE << " allocated slots";
	}
#endif
};

#endif
