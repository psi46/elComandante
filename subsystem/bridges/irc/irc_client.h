/**
 * \file irc_client.h
 * \author Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */
#ifndef IRC_CLIENT_H
#define IRC_CLIENT_H

#include <string>

/**
 * Maximum length of the messages
 */
#define MSG_LENGTH	512

/**
 * \name Return values for the different IRC-messages
 */
//@{
#define IRC_PRIVMSG	0
#define IRC_PING	1
#define IRC_OTHER	2
//@}

/**
 * \brief small irc client with basic functions
 *
 * This class is a very small irc-client, witch supports only the basic fucntions, like writing a message, 
 * joining and quiting a channel and receiving a message. All messages in the channel are send to this client.
 */
class irc_client {

	private:
		int socket_fd;
		char msg[MSG_LENGTH];
		char buffer[MSG_LENGTH*2];

	public:
		irc_client(char* hostname);
		~irc_client();

		int recv_packet(int &type, std::string &irc_msg);

		int join(char* channel);
		int send_msg(char* msg);
		int change_nick(char* name); // change nick_name
		int quit();

		int getfd() { return socket_fd; };  // return filedescriptor for select
};


#endif //ndef IRC_CLIENT_H
