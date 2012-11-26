/**
 * \file sclient_thread.h
 * \author Dennis Terhorst
 * \date Mon Nov  9 22:01:43 CET 2009
 */
#ifndef SCLIENT_THREAD_H
#define SCLIENT_THREAD_H

#include "globals.h"
#include "thread_t.h"

/** \brief class wrapper for the sclient thread
 *
 * this class is supposed to be run with the thread_t::run() wrapper
 */
class sclient_thread : public thread_context_t {
public:
	int run() {
		action_context* ac = global.context;
		sclient_selectable me;
		if (!me.isOK()) {
			return -1;
		}
		me.setid("script_test");
		me.setDefaultSendname("/script/errors");
		me.printf("subscript(sclient): started\n");

		global.scptr = &me;	// make this sclient public

		packet_t packet;
		int ret;
		while (global.scptr!=NULL) {
			switch ( selectable::run(1,0) ) {
			case 0:	// timeout
				break;
			case -1:
				// select error
				me.aprintf("/errors", "subscript(sclient): select() error!\n");
				break;
			default:
				ret = me.recvpacket(packet);
				if ( ret < 0 ) {
					global.scptr = NULL;	// die
					cerr << "sclient returned an error: " << strerror(errno) << endl;
				} else if (ret > 5) {
					string name = string(packet.name(), packet.namelen()-1);// cut \0 termination
					string data = string(packet.data(), packet.datalen());
					//ac->var_set(name, data);
					try {
						ac->pkt_parse(name, data, (packet_type_t)packet.type); // FIXME: cast should not be neccessary
					}
					catch (errno_exception<EBADMSG> &e) {
						me.printf("subscript(sclient): rx: parsing %s data \"%s\" failed %s\n",
								 name.c_str(), data.c_str(), e.what());
					}
					catch (errno_exception<EPROTO> &e) {
						me.printf("subscript(sclient): rx: parsing %s data failed %s\n",
								 name.c_str(), e.what());
					}
				} else {
					me.printf("subscript(sclient): strange rx: returned %d\n", ret);
				}
			}
		}
		
		global.scptr = NULL;	// unpublish sclient
		me.printf("subscript(sclient): thread exit.\n");
		return 0;
	}

	void abort() { global.scptr=NULL; }
};



#endif //ndef SCLIENT_THREAD_H
