/**
 *	\file abo.h
 *	\author Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 *	\date Mon Feb 11 10:02:05 CET 2008
 */
#ifndef ABO_H
#define ABO_H

#include <time.h>

//#include "clientinfo.h"
class clientinfo;

#include "packet_t.h"
#include "logfile_t.h"
#define MAX_NAMELEN	256
//#define MAX_SUPPLIERS	10
//#define MAX_SUBSCRIBERS	32
#include "slist.h"
#define MAX_ABOS	128
// NOTE: MAX_ABOS is an untested limit, because constructor cannot abort!

// abo flags
#define AF_LOG		1
#define AF_ADDTIME	2
#define AF_UNIQSUPPLY	4
// AF_LOG		not used currently, but should substitute loglevel (FIXME)
// AF_ADDTIME		server will prepend unix timestamp to all lines of data
// AF_UNIQSUPPLY	the server will not accept more than one supplier
//			for this abo. (FIXME: not jet implemented)

/**
 * \brief keeps information about one abo
 *
 * Store abo specific data like name, flags and logging. This class is the
 * couterpart of clientinfo and together the two classes make a n:m liked
 * structure which client has subscribed which abo and vice versa.
 *
 * \sa clientinfo
 */
class abo {
private:
	int loglevel;
	unsigned int flags;
	logfile_t* mylog;

	int myindex;	///< index of this abo in class list
public:
	static abo* list[MAX_ABOS];
	static int noabos;

	char name[MAX_NAMELEN];
	int namelen;	// FIXME besser kapseln
	time_t	lastrecv;

	/// \name clientlinks clientinfo lists
	///
	/// These lists hold pointers to the clients which have subscribed
	/// this abo or are supplying it.
	//@{
	slist<clientinfo*>	supplier;
	slist<clientinfo*>	subscriber;
	//@}

	// constructor
	abo(char* aboname, int abonamelen);

	// destructor
	~abo();

	///
	/// \name client management
	///
	//@{
	/// return number of suppliers of this abo
	int Suppliers();
	/// add client to list of suppliers of this abo
	int addSupplier(clientinfo* suppl);
	/// remove client from list of suppliers of this abo
	int removeSupplier(clientinfo* suppl);

	/// return number of subscribers of this abo
	int Subscribers();
	/// add client to list of subscribers of this abo
	int addSubscriber(clientinfo* subscr);
	/// remove client from list of subscribers of this abo
	int removeSubscriber(clientinfo* subsrc);
	//@}

	/**
	 * get abo with name 'aboname' from the list[]
	 * or create a new abo
	 */
	static abo* Abo(char* aboname, int abonamelen) {
		for (int i = 0; i<noabos; i++) {
			if ( (strcmp(aboname, list[i]->name) == 0)
			  //&& (abonamelen == list[i]->namelen) ) {	// FIXME: some calls seem to have wrong namelen
			  ) {
				return list[i];
			}
		}
		return new abo(aboname, abonamelen);
	}


	/// \name traffic stats
	//@{
	void received();
	//@}
	
	/// \name Logging
	//@{
	int getLogLevel();
	int setLogLevel(int n);
	int logPacket(const packet_t& packet);
	//@}

	/// \name Flags
	//@{
	void setFlag(unsigned int add);
	void setFlags(unsigned int to);
	void delFlag(unsigned int del);
	unsigned int getFlags();
	//@}

	/**
	 * get the internal index in list[]
	 */
	int index();
	
	/// \name informational & debug
	//@{
	/// print a abolist to stdout
	/// \deprecated Storage classes must not use printf functions
	static void printAbos();

	//@}
};

#endif
