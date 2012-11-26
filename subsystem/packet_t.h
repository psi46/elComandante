/**
 * \file packet_t.h
 * \author Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 * \date 14 Feb 2008
 */
#ifndef PACKET_T_H
#define PACKET_T_H

#ifdef _WIN32	// WINDOWS
	#include <winsock.h>
	#include <winsock2.h>
	#include <io.h>

	#define socklen_t int	//FIXME not nice
#else			// LINUX
	#include <sys/socket.h>	// socket
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>	// inet_aton
#endif

#include <stdarg.h>	// used in pprintf

#include <string.h>


/**
 * \name Packet Type Definitions
 *
 * These are constant values for the packet_t::type field.
 */
//@{
/// enum parameter packet_t::type
#ifdef USE_PACKET_TYPE_ENUM
typedef enum {
	PKT_MANAGEMENT	=0,
	PKT_DATA	=1,
	PKT_SUBSCRIBE	=2,
	PKT_UNSUBSCRIBE =3,
	PKT_SUPPLY	=4,
	PKT_UNSUPPLY	=5,
	PKT_CLIENTTERM	=6,
	PKT_SERVERTERM	=7,
	PKT_SETDATA	=8
} packet_type_t;
#else
/// OBSOLETE: packet_t::type field preprocessor constants
#define PKT_MANAGEMENT	0
#define PKT_DATA	1
#define PKT_SUBSCRIBE	2
#define PKT_UNSUBSCRIBE 3
#define PKT_SUPPLY	4
#define PKT_UNSUPPLY	5
#define PKT_CLIENTTERM	6
#define PKT_SERVERTERM	7
#define PKT_SETDATA	8
#define PKT_DEFAULTTYPE	PKT_DATA	///< Default packet type set in construtor
#endif //def USE_PACKET_TYPE_ENUM
//@}


/**
 * PKT_HEADLEN must be EVEN due to 16-bit memory alignment!!
 * should actually be (sizeof(type)+sizeof(namelength))
 */
#define PKT_HEADLENGTH	4

/**
 * \brief maximum raw packet length
 *
 * This value must be as large as possible to hold all data packets to be transmitted,
 * but is constrained by IP MTU (see <A HREF="http://www.faqs.org/rfcs/rfc791.html">IP</A>
 * and <A HREF="http://www.faqs.org/rfcs/rfc768.html">UDP</A>).
 *
 * \warning Limits and performance has not been tested jet. Packets with
 * lenths close to this value may cause severe problems in some places.
 */
#define MAX_PACKETLEN	2047

/**
 * \brief subsystem packet definiton
 *
 * This class defines the structure of the underlaying packets transmitted
 * to/from a subserver. Basically this is a packet name string combined with
 * a data payload. The name shall not contain line breaks, data may be ascii
 * or binary. (binary not tested throughly jet).
 *
 * \todo Test packet_t with binary data and long packet lengths.
 */
class packet_t {
public:
	static int MAX_LENGTH;
	int		length;
	union {
		unsigned char	raw[MAX_PACKETLEN];	///< length of the whole packet
		struct {
			unsigned short	type;
			unsigned short	namelength;				// PRIVATE
			unsigned char	alldata[MAX_PACKETLEN-PKT_HEADLENGTH];	// PRIVATE
		};
	};
	struct sockaddr_in	address;		///< destination or source address of the packet, depending on context
	socklen_t	addrlen;

	packet_t();

	/**
	 * \brief check packets general structure
	 *
	 * currently this only checks if there is a string termination char '\\0' at
	 * position \c namelength in the buffer.
	 */
	bool	isValid() const;

	/**
	 * \brief set target IP address
	 */
	bool	setAddress(struct sockaddr_in addr, socklen_t len);

	/**
	 * \name Packet Name Functions
	 */
	//@{
	/**
	 * \brief set the name field of packet
	 *
	 * copies the \c buffer to the beginning of the packet adjusting
	 * \p namelength accordingly. After a call to setName() the data
	 * of the packet is invalid.
	 */
	int	setName(const char* buffer);

	/**
	 * \brief return a pointer to the packets name
	 *
	 * The string the returned pointer points to must not be changed,
	 * as it will break the internal \c namelenght accounting.
	 *
	 * \todo packet_t::name() function shall return a const char*
	 */
	char*	name() const;	// FIXME: return const char*
	/// gives current \c namelength
	int	namelen() const;
	//@}
	
	/**
	 * \name Packet Data Functions
	 */
	//@{
	/**
	 * \brief set the data field of packet
	 *
	 * copies the \c buffer to the beginning of the packets data space
	 * adjusting all lengths accordingly. After a call to setName() this
	 * field is invalid! You must call setData() after a setName() call.
	 */
	int	setData(const char* buffer, const int len);
	/**
	 * \brief return a pointer to the packets data
	 *
	 * The contents the returned pointer points to must not be changed
	 * in length, as it will break the internal lenght accounting.
	 *
	 * \todo packet_t::data() function shall return a const char*
	 */
	char*	data() const;
	/// gives current data field length
	int	datalen() const;
	/**
	 * \brief set data field of a packet
	 *
	 * This function can be used like a printf to the packets data
	 * field and is very useful for ascii data.
	 */
	int	pprintf(const char* format, ...);
	//@}

	/// gives the IP address of the packet as string
	char*	addrString() const;

	/**
	 * \brief print packet to stdout
	 *
	 * If packets only contain ascii data one might want to use this
	 * function for debugging purposes. Nevertheless this is not
	 * considdered to be good practice.
	 *
	 * \deprecated Storage classes shall not use printf functions.
	 */
	void	print() const;

};

/*
#define MAX_INFOLEN	512
class packetinfo {
public:
	int		len;
	packet_t	packet;
};
*/

//##################################################################################
//##################################################################################

//##################################################################################
//##################################################################################



#endif
