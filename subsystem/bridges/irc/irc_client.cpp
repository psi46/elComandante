#include "irc_client.h"	

#include <iostream>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <string>

#include "error.h"

char *substr (const char *str, int start, int end) {
    char *sub = new char[strlen(str)];
    start -=1 ;

    for(int i = 0;start<end;i += 1) {
            sub[i] = str[start];
            start += 1;
    }
    sub[end] = '\0';
    return sub;
}

irc_client::irc_client(char* hostname){
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("failure while creating irc_socket!");
		exit(1);
	}
	struct sockaddr_in server_addr; 
	server_addr.sin_family = AF_INET;    
	server_addr.sin_port = htons(6667);  
	server_addr.sin_addr.s_addr = inet_addr(hostname); 
	if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
		perror("failure  while connecting to irc");
		exit(1);
	}
	// Log on server with NICK
	sprintf(msg,"USER subserver subserver subserver :SubserverBot\r\n");
	if (send(socket_fd, msg, strlen(msg), 0) == -1) {
		perror("send");
	}
	sprintf(msg,"NICK IRC-SUBSERVER\r\n");
	if (send(socket_fd, msg, strlen(msg), 0) == -1) {
		perror("send");
	}
	// Join test channel
	sprintf(msg, "join :#testchan\r\n");
	if (send(socket_fd, msg, strlen(msg), 0) == -1) {
		perror("send");
	}	
}

irc_client::~irc_client(){
	close(socket_fd);
}
//int irc_client::recv_packet(const char* my_msg, const char* my_user, const char* my_channel, int &type){
int irc_client::recv_packet(int &type, std::string &irc_msg){
	int length = 0;

	length = recv(socket_fd, msg, MSG_LENGTH-1, 0);
	
	if(strstr(msg, "PING")){
		type = IRC_PING;
		std::string msg_string(msg);
		int i = msg_string.find("PING");
		sprintf(buffer, "PONG %s\r\n",substr(msg,i+6,i+13));
		send(socket_fd, buffer, strlen(buffer), 0);
		irc_msg = "";
		return 0;
	}else if(strstr(msg, "PRIVMSG")){
		type = IRC_PRIVMSG;

		std::string msg_string(msg);
		msg_string = msg_string.substr(0,msg_string.find('\n'));

//		printf("-> sender: %s\n",msg_string.substr(1,msg_string.find('!')-1).c_str());
		//my_user = msg_string.substr(1,msg_string.find('!')-1).c_str();
		msg_string = msg_string.substr(msg_string.find('!')+1, std::string::npos);

//		printf("-> sender addr: %s \n",msg_string.substr(0,msg_string.find("PRIVMSG")-1).c_str());
		msg_string = msg_string.substr(msg_string.find("PRIVMSG")+8,std::string::npos);

//		printf("-> channel: %s\n", msg_string.substr(0,msg_string.find(':')-1).c_str());
		//my_channel = msg_string.substr(0,msg_string.find(':')-1).c_str();
		msg_string = msg_string.substr(msg_string.find(':')+1,std::string::npos);	

//		printf("-> message: %s\n", msg_string.c_str());

		sprintf(msg, msg_string.c_str());
		irc_msg = msg_string;
		return strlen(msg);
	}
	return length;
}
int irc_client::send_msg(char* msg){
	char buffer[MSG_LENGTH];
	sprintf(buffer, "PRIVMSG #testchan :%s\r\n", msg);
	return send(socket_fd, buffer, strlen(buffer), 0);
}
int irc_client::quit(){
	// send bye! to #testchan, then quit
	sprintf(msg, "PRIVMSG #testchan :Subserver exiting!\r\nQUIT\r\n");
	printf("IRC-quit\n");
	return send(socket_fd, msg, strlen(msg), 0);
}

