/**
 * \file selectable_sclient.h
 * \author Dennis Terhrost
 * \date April 2009
 */
#ifndef SELECTABLE_SCLIENT_H
#define SELECTABLE_SCLIENT_H

#include "sclient.h"
#include "selectable.h"

class sclient_selectable : public selectable, public sclient {
public:
	sclient_selectable() : selectable(),sclient() {};
	virtual ~sclient_selectable() {};

	// selectable virtuals
	virtual int getfd() const;
	virtual int getchecks() const;
};

#endif //ndef SELECTABLE_SCLIENT_H
