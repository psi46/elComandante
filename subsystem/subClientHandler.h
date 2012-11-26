/*
 * subClientHandler.h
 *
 *  Created on: 27.02.2012
 *      Author: Felix Bachmair
 */

#ifndef SUBCLIENTHANDLER_H_
#define SUBCLIENTHANDLER_H_
#include <vector>
#include <list>
#include <queue>
#include <set>
#include "selectable_sclient.h"
//#include <error.h>
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <string>

class subClientHandler {
    
public:

	subClientHandler(std::string clientName="subSystemClient");
	virtual ~subClientHandler();
	enum sendType{SETDATA=0,DATA=1,};
	void sendAndReceive();
private:
    void echoToPing(std::string aboname);
    std::string clientName;
public:
	//special packet format where the timestamp has been extracted
	struct packetData_t{
		int timeStamp;
		std::string aboName;
		std::string data;
	};
protected:
	virtual void  repeatedActions(){};
	float timeout;
	bool bKillClient;
	bool isKilled;
	//client for communication with subsystem
	sclient_selectable *subServerClient;
	//queue of all received packets which have to be analysed
	std::queue<packet_t> receivedPackets;
	//all subscribed Abos are saved in this set
	std::set<std::string> subscribedAbos;
	//all suppliedAbos are saved in this set
	std::set<std::string> suppliedAbos;

	//packets where the timestamp has been extracted
	std::queue<packetData_t> dataQueue;
public:
	bool isClientKilled(){return isKilled;}
	void killClient(){bKillClient=true;std::cout<<"kill clientHandler"<<bKillClient<<std::endl;};
	/**
	 *  checks is connection with subserver is working all right
	 *  \return true if subsystem works all right
	 *  \return false if client cannot connect to subserver
	 */
	bool isOk(){if(bKillClient)std::cout<<"client should be killed"<<std::endl;return subServerClient->isOK();}
	/**
	 *  sets client name for the connection with the subserver
	 *  \param clientName name of client
	 */
	void setClientName(std::string clientName);

	/**
	 * receives all subscribed abos while programm is running
	 * analyse this abos and updates all variables
	 */
	void* receiveAbos(void*);
	/**
	 * unsubscribe all subscribed abos
	 */
	void unsubscribeAll();
	/**
	 * unsupplys all supplied abos
		 */
		void unsupplyAll();
		/** supply abo of name aboName
		 * \param aboName name of abo you want to supply
		 */
		void supplyAbo(std::string aboName);
		/**
		 * subscribes alls Abos which are in the vecAbos
		 * \param vector of abos you want to subscribe
		 */
		void setSubscribedAbos(std::vector<std::string> vecAbos);
		/**
		 * subscribes abo of name aboName
		 * \param aboName name of abo you want to subscribe
		 */
		void subscribeAbo(std::string aboName);

		/**
		 * closes the connection to subsystem and unsupply and
		 * unsubscribe all abos
		 */
		void closeConnection();
		void setTimeOut(unsigned int timeout){this->timeout=timeout;std::cout<<"timeout = "<<timeout<<std::endl;}
        bool getReceivedPacket(packet_t &receivedPacket);
        void getAboFromServer();
		bool extractData(packet_t packet);
		void saveData(packetData_t data);
		bool getData(packetData_t &data);
		virtual bool analyseData(packetData_t data) {return false;};
		void unsupplyAbo(std::string aboName);
		void unsubscribeAbo(std::string aboName);
		/**
		 * \brief sends data to abo aboName
		 * checks if connection to subsystem is ok
		 * checks if abo is supplied or not, when not it supllies the abo
		 * adds a timestamp in front of the data
		 * sends data to given abo aboName
		 * \param aboName name of abo to send to
		 * \param data data which should be send, without timestamp
		 */
		void sendToServer(std::string aboName,std::string data);
		void set(std::string aboName, std::string key, float value);

		void sendError(std::string errorMessage);

};

//
//using namespace boost::python;
//
//// Boost.python definitions to expose classes to Python
//BOOST_PYTHON_MODULE(subClientHandler) {
//    class_<subClientHandler> ("subClientHandler")
//        .def_readwrite("value", &Node::value)
//        .def("receiveAbo", &subClientHandler::receiveAbo)
//    ;
//}

#endif /* SUBCLIENTHANDLER_H_ */
