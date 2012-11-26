/*
 * \file subserver.h
 * \brief subscription server main class
 * \author Dennis Terhorst
 * \date 14 Mar 2008
 */
#ifndef SUBSERVER_H
#define SUBSERVER_H

/**
 * \name Default Connection Parameters
 */
//@{
/// Environment variable name where the server IP might be defined
// set local address to one of INADDR_ANY, INADDR_LOOPBACK or give ip string 
//#define MY_ADDR		"127.0.0.1"
//#define MY_ADDR		"134.61.14.103"
#define SUBSERVER_ENVNAME     "SUBSERVER"
/// Server address to resort to if all other means of identification fail
#define SUBSERVER_DEFAULT_ADDR	"127.0.0.1"
/// Server port to resort to if all other means of identification fail
#define SUBSERVER_DEFAULT_PORT	12334
//@}

/// environment read buffer
#define ENVBUFLEN	512

/// \name stderr verbosity
//@{
#define NONVERBOSE	0
#define SHOW_PACKETS	1
#define SHOW_PACKET_DROPS	2
#define VERBOSE		(NONVERBOSE)
//@}

//#define MY_PORT		12334
#include "logfile_t.h"
#include "slist.h"
#include "command_t.h"
#include "clientinfo.h"
#include "abo.h"
#include "packet_t.h"


#include "CPU_info.h" // Get CPU Info

/// template specification used with command_t.h
#define SUBSERVER_CMDFUNC int (subserver::*)(clientinfo* client, int argc, char* argv[])

/**
 * \brief subscription server main class
 *
 * This is the central part of the subsystem. A subserver listens on a
 * network socket and accepts UDP packets from all clients.
 *
 * Using a subscription model it distributes the information received
 * to all clients which have subscribed the particular abonement. The
 * information distribution model based upon ideas from IP Multicasting
 * (<A HREF="http://www.faqs.org/rfcs/rfc1054.html">RFC1054</A>,
 * <A HREF="http://www.faqs.org/rfcs/rfc1075.html">RFC1075</A>).
 *
 * Using packets with type \c PKT_MANAGEMENT the behaviour of the server
 * can be changed at runtime. Logging can be enabled or disabled and
 * broken clients or abos can be removed from internal list.
 * The "help" command lists commands currently available.
 *
 * For clients providing non-timestamped information the server can
 * also prepend a default unix time timestamp to the data fields
 * of packets of specific abos. This is particularly useful for logging
 * user input from clients not timestamping their packets.
 *
 */
class subserver {
private:

	CPU_info* myCPUinfo;

	int sfd;	// socket file descriptoer
	struct sockaddr_in myaddr;

	//
	// PKT_MANAGEMENT facilities
	//
	slist< command_t<SUBSERVER_CMDFUNC> > cmd;	// list of known mgm commands


	//
	// constructor's helper
	//
	int init(const char* serverip, const short serverport) ;
public:
	subserver() ;

	subserver(const char* serverip, const int serverport) ;

	~subserver() ;
	
	int getfd() const ;
	bool isOK() const ;

	int sendpacket(packet_t& packet) ;

	int recvpacket(packet_t& packet) ;

	void terminate() ;

	//static void parse(char* text, int textlen, int& argc, char* argv[]) ;

	void do_mgm_packet(packet_t* packet) ;

	
	/**
	 * \name SUBSERVER MANAGEMENT FUNCTIONS
	 * 
	 * Arriving packets with type \c PKT_MANAGEMENT will be parsed by
	 * command_t::parse() and the corresponding command will be
	 * executed.
	 */
	//@{
	
	/// this function just returns a "unknown command" response to
	/// the client which has sent the packet
	int unknowncommand(clientinfo* client, int argc, char* argv[]) ;

	int senderror(clientinfo* client, const char* format, ...) ;

	/**
	 * \brief initialize the command list
	 *
	 * This function is called once from the constructor to build
	 * the list with commandname-to-function links (see command_t)
	 */
	void init_commands() ;

	int cmd_listclients(clientinfo* client, int argc, char* argv[]) ;
	int cmd_listgraphical(clientinfo* client, int argc, char* argv[]) ;
	int cmd_removeclients(clientinfo* client, int argc, char* argv[]) ;
	int cmd_listabos(clientinfo* client, int argc, char* argv[]) ;
	int cmd_removeabos(clientinfo* client, int argc, char* argv[]) ;
	int cmd_setClientID(clientinfo* client, int argc, char* argv[]) ;
	int cmd_log(clientinfo* client, int argc, char* argv[]) ;
	int cmd_nolog(clientinfo* client, int argc, char* argv[]) ;
	int cmd_time(clientinfo* client, int argc, char* argv[]) ;
	int cmd_notime(clientinfo* client, int argc, char* argv[]) ;
	int cmd_unique(clientinfo* client, int argc, char* argv[]) ;
	int cmd_nounique(clientinfo* client, int argc, char* argv[]) ;
	int cmd_help(clientinfo* client, int argc, char* argv[]) ;
	int cmd_restart(clientinfo* client, int argc, char* argv[]) ;
	int cmd_health(clientinfo* client, int argc, char* argv[]); 
	//@}
	
}; // end of class subserver


#endif
