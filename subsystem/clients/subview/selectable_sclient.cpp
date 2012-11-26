/**
 * \file selectable_sclient.cpp
 * \author Dennis Terhrost
 * \date April 2009
 */
#include "selectable_sclient.h"

//
// selectable virtuals
//
int sclient_selectable::getfd() {
	return this->sclient::getfd();
}

int sclient_selectable::getchecks() {
	return CHK_READ;
}


