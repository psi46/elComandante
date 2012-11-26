/**
 * \file selectable_sclient.cpp
 * \author Dennis Terhrost
 * \date April 2009
 */
#include "selectable_sclient.h"

//
// selectable virtuals
//
int sclient_selectable::getfd() const {
	return this->sclient::getfd();
}
//	return -1;
//}
int sclient_selectable::getchecks() const {
	return CHK_READ;
}


