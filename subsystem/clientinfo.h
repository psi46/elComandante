/**
 *	\file clientinfo.h
 *	\author Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 *	\date Mon Feb 11 10:02:05 CET 2008
 */
#ifndef CLIENTINFO_H
#define CLIENTINFO_H

#include <time.h>
#ifdef _WIN32
	#include <winsock2.h>
#else
	#include <sys/socket.h>	// socket
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>	// inet_aton
#endif

#include "slist.h"
//#include "abo.h"
class abo;

#define MAX_SUPPLIES		10
#define MAX_SUBSCRIPTIONS	10
#define MAX_CLIENTINFOS		128
// NOTE: MAX_CLIENTINFOS is an untested limit, because constructor cannot abort!
#define MAX_IDLEN		128

/**
 * \brief keeps information about one client
 *
 * Store abo specific data like name, flags and some statistics. This class is
 * the couterpart of abo and together the two classes make a n:m liked
 * structure which client has subscribed which abo and vice versa.
 *
 * \sa abo
 */

class clientinfo {
private:
	int myindex;		///< index of this clientinfo in class list
public:
	static clientinfo*	list[MAX_CLIENTINFOS];
	static int		noclientinfos;
	char id[MAX_IDLEN];

	time_t	lastrecv;
	time_t	lastsend;
	float	ppsrecv;	// packets per second
	float	ppssend;	// packets per second
	struct sockaddr_in	address;	// client address in net byte order

	/// \name abo lists
	///
	/// These lists hold pointers to the abos which have been subscribed
	/// by this client or are supplied by it.
	//@{
	slist<abo*>	supply;
	slist<abo*>	subscription;
	//@}

	// constructor
	clientinfo(struct sockaddr_in *addr);

	// destructor
	~clientinfo();

	///
	/// \name abo management
	///
	//@{
	/// return number of supplied abos
	int Supplies();
	/// Add suppl to list of supplied abos
	int addSupply(abo* suppl);
	/// Remove suppl from list of supplied abos
	int removeSupply(abo* suppl);	

	/// return nuber of subscriptions
	int Subscriptions();
	/// add subscr to list of subsrcibed abos
	int addSubscription(abo* subscr);
	/// remove subscr from list of subscribed abos
	int removeSubscription(abo* subscr);
	//@}
	
	/**
	 * get clientinfo describing addr from the list[]
	 * or create a new clientinfo
	 */
	static clientinfo* Client(struct sockaddr_in *addr) {
		for (int i = 0; i<noclientinfos; i++) {
			if ( (addr->sin_addr.s_addr == list[i]->address.sin_addr.s_addr)
			  && (addr->sin_port == list[i]->address.sin_port)) {
				return list[i];
			}
		}
		return	new clientinfo(addr);
	}

	/// \name traffic stats
	//@{
	void received();	///< update lastrecv time
	void sent();		///< update lastsend time
	//@}

	/// \name informational & debug
	//@{
	
	/// set an alternative name for this client
	/**
	 * This function just holds a string for this client to help identifying
	 * the actual process behind the network link.
	 */
	void setID(char* name);

	/// get the alternative name of this client
	const char* getID();

	/// informational & debug
	char* addrString();

	/**
	 * get the internal index in list[]
	 */
	int index();

	/// print a clientlist to stdout
	/// \deprecated Storage classes must not use printf functions
	static void printClients();
	//@}
};

#endif
