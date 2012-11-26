/**
 * \file sclient.h
 * \author Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 * \date 18.Feb.2008
 */
#ifndef SCLIENT_H
#define SCLIENT_H

#include <sys/types.h>
#include <stdarg.h>

#include "packet_t.h"

/**
 * \name Default Connection Parameters
 */
//@{
/// Environment variable name where the server IP might be defined
#define SUBSERVER_ENVNAME	"SUBSERVER"
/// Server address to resort to if all other means of identification fail
#define SUBSERVER_DEFAULT_ADDR	"127.0.0.1"
/// Server port to resort to if all other means of identification fail
#define SUBSERVER_DEFAULT_PORT	12334
//@}

/**
 * \defgroup sc_callback Packet Receive Callback Mechanism
 *
 * The Callback mechanism can be used to call stored function call
 * pointers upon reception of a packet with a specific abo name. By
 * this an easy means for pre-sorting multiple subscriptions is given.
 */
//@{

/// Maximum number of callback functions defineable
#define MAX_CALLBACKS	32
///////////////////////////////////////////////////
#include <string.h>
/**
 * \brief subscription callback abstraction
 *
 * This class links a <TT>char* aboname</TT> to a *function(packet_t)
 * pointer which is called by the run() function. All members are public
 * as this is just like a more intelligent \c struct to keep data in.
 *
 * \deprecated This class has not been used a lot, because very often
 * 	there are more specific needs.
 */
template <class ret_t>
class rxcall_t {
public:
	char abo[MAX_PACKETLEN];
	ret_t (*func)(packet_t);

	rxcall_t(const char* aboname, ret_t (*pfunc)(packet_t packet)) {
		strncpy(abo, aboname, MAX_PACKETLEN);
		func = pfunc;
	}

	ret_t run(packet_t packet) {
		return func(packet);
	}
}; // end class rxcall_t
///////////////////////////////////////////////////
//@}

/**
 * \brief subsystem client main class
 *
 * This class provides the connection to a subserver and by that can be
 * seen as the key class of the subsystem architecture.
 *
 * Upon instanciation this class tries to find a server address to connect
 * to by using the given \p serverip and \p serverport, the environment
 * variable with name SUBSERVER_ENVNAME or the default address.
 *
 * If connected successfully this class does all the socket handling
 * needed to transfer packets to and from the subserver. It can be used
 * in various ways with blocking read calls or by using getfd() with
 * select() or poll() calls for multiplexed i/o operations.
 */
class sclient {
private:
	/// socket file descriptor
	#ifdef _WIN32
    		SOCKET sfd;
	#else
		int sfd;
	#endif
	struct sockaddr_in svraddr;	///< server address structure

	/**
	 * \brief connect to the subserver
	 */
	void init(const char* serverip, const short serverport);

	/**
	 * \brief default abo name
	 *
	 * Holds the default name used with printf() and set() functions.
	 */
	char DefaultSendname[MAX_PACKETLEN];

	/**
	 * \brief storage of callback definitions
	 * \ingroup sc_callback
	 */
	rxcall_t<void>* rxcallback[MAX_CALLBACKS];
	int norxcallbacks;	///< \ingroup sc_callback
public:
	sclient();
	sclient(const char* serverip, const short serverport);

	~sclient();

#ifdef _WIN32
	SOCKET getfd();
#else
	int getfd() const;
#endif
	/// check connection for errors
	bool isOK();

	/**
	 * \brief send the packet to the subserver
	 */
	int sendpacket(packet_t& packet);

	/**
	 * \brief receive a packet from the subserver
	 *
	 * This function blocks until a packet is received. If this behaviour
	 * is not appreciated use select() or poll() functions to anticipate
	 * the arrival of a packet with timeout possibilities.
	 */
	int recvpacket(packet_t& packet);

	/**
	 * \brief set a client identifier
	 *
	 * Call this function to announce a name for this client instance
	 * to the server. This helps a good deal in finding the sources of
	 * packets flying around in the subsystem.
	 */
	void setid(const char* name);

	/**
	 * \brief subscribe an abo from the subserver
	 */
	int subscribe(const char* name);

	/**
	 * \brief retract a subscription of an abo
	 */
	int unsubscribe(const char* name);

	/**
	 * \brief announce to provide an abo to the subserver
	 *
	 * Although the server registers any client sending a PKT_DATA packet as supplier of that specific abo,
	 * this function provides a way to get PKT_SETDATA packets without sending a dummy data packet.
	 */
	int supply(const char* name);

	/**
	 * \brief announce not to provide an abo to the subserver anymore
	 */
	int unsupply(const char* name);
	
	/**
	 * \name Ascii Send Functions
	 *
	 * These functions provide convenient methods to send ascii packets.
	 * In contrast to a sendpacket() call no packet_t has to be build in 
	 * the users program. Instead the packet is constructed in these
	 * functions and is then automatically sent to the subserver.
	 */
	//@{
	/**
	 * \brief set the default abo name for sent packets
	 *
	 * The name given here will be used for all packets build with
	 * the printf() and set() functions. 
	 */
	int setDefaultSendname(const char* name);
	
	/**
	 * \brief print to the subserver
	 *
	 * Just use this function like a normal printf() call and all text
	 * will be written to a packet (named as given with setDefaultSendname() )
	 * and sent to the subserver.
	 * \li packet.type = \c PKT_DATA
	 * \li packet.name = \c DefaultSendname
	 */
	int printf(const char* fmt, ...);

	/**
	 * \brief print to the subserver or abo \c name
	 *
	 * Just like printf() this function provides a convenient way for
	 * ascii data output. In contrast to printf() this function will
	 * use the abo \c name given, instead of the DefaultSendname.
	 * \li packet.type = \c PKT_DATA
	 * \li packet.name = \p name
	 */
	int aprintf(const char* name, const char* fmt, ...);

	/**
	 * \brief print to the subserver for abo \c name
	 *
	 * This function creates a packet with type \c PKT_SETDATA with
	 * a preliminary set data syntax and transmitts this packet
	 * to the subserver.
	 * \li packet.type = \c PKT_SETDATA
	 * \li packet.name = \p name
	 * \li packet.data = &quot;set <I>key value</I>&quot;
	 *
	 * \todo Clean PKT_SETDATA data() structure definiton.
	 */
	int set(const char* name, char* key, double value);

	//@}
	
	/**
	 * \brief callback function registration
	 * \ingroup sc_callback
	 */
	int register_rxcall(void (*func)(packet_t), const char*aboname);
	/**
	 * \brief callback function execution
	 * \ingroup sc_callback
	 */
	void do_rxcalls(packet_t packet);

	/// terminate connection to the subserver
	void terminate();
};


#endif
