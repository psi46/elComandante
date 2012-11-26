/**
 * \file exceptions.h
 * \author Dennis Terhorst
 * \date Tue Oct  6 18:21:32 CEST 2009
 */
#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>

/** \brief general exception class */
class general_exception : public std::exception {
	std::string text;
	int err;
public:
	general_exception(std::string newtext,int errnumber=0) throw() : std::exception() {
		text = newtext;
		err = errnumber;
	};

	virtual const char* what() const throw() {
		return text.c_str();
	}
	virtual ~general_exception() throw() {};
};

#include <errno.h>
#include <string.h>
#include <string>

/** \brief exception class template for errno types */
template <int ERRNO=0>
class errno_exception : public general_exception {
public:
	errno_exception(std::string newtext="",int errnumber=ERRNO) throw()
		: general_exception((newtext==""?"":newtext+": ")+strerror(errnumber), errnumber)
	{ }
};


#endif //ndef EXCEPTIONS_H
