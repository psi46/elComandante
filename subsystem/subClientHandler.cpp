/*
 * subClientHandler.cpp
 *
 *  Created on: 27.02.2012
 *      Author: Felix Bachmair
 */

#include "subClientHandler.h"

using namespace std;
subClientHandler::subClientHandler(std::string clientName) {
	// TODO Auto-generated constructor stub

    subServerClient = new sclient_selectable();
    subServerClient->setid(clientName.c_str());
    subServerClient->isOK();
    subServerClient->setDefaultSendname(clientName.c_str());
    this->clientName = clientName;
    stringstream output;
    output<<"Started client: \""<<clientName<<"\"";
    this->sendToServer("/log",output.str());
    this->isKilled=false;
    this->bKillClient=false;
    timeout=-1;
}

subClientHandler::~subClientHandler() {
	// TODO Auto-generated destructor stub
}

void subClientHandler::echoToPing(std::string aboName){
    std::stringstream data;
    data<<":Pong! "<<this->clientName;
    this->sendToServer(aboName,data.str());
}
void subClientHandler::sendToServer(std::string aboName, std::string data){
	if(subServerClient->isOK()){
		int timeStamp = time(NULL);
		std::string sendData;
		std::stringstream sStream;
		sStream<<timeStamp;
		sStream<<" ";
		sStream<<data;
		sStream<<std::endl;
		this->supplyAbo(aboName);
		this->subServerClient->aprintf(aboName.c_str(),sStream.str().c_str());
	}
	else
		std::cerr<<"Client not OK! Please check SUBSERVER!!!"<<std::endl;
}

void subClientHandler::set(std::string aboName, std::string key, float value){
	if(subServerClient->isOK()){
			int timeStamp = time(NULL);
			std::string sendData;
			std::stringstream sStream;
			sStream<<timeStamp;
			sStream<<" ";
			sStream<<key;
			sStream<<std::endl;
			this->supplyAbo(aboName);
			//this->subServerClient->set(aboName.c_str(),sStream.str().c_str()),value);
//			TODO
//			this->subServerClient->set(aboName.c_str(),"bla",value);
	}

	else
		std::cerr<<"Client not OK! Please check SUBSERVER!!!"<<std::endl;
}

bool subClientHandler::getData(packetData_t & data)
{
	if(this->dataQueue.empty())
		return false;
	else {
		data = this->dataQueue.front();
		this->dataQueue.pop();
		//std::cout<<"got data"<<std::endl;
		return true;
	}
}


void subClientHandler::sendAndReceive(){
	std::cout<<"SEND AND RECEIVE FUNCTION"<<endl<<endl;
	int selret;
	packet_t tmpPacket;
	if (this->subServerClient!=NULL)
		while(!bKillClient){
			if(!subServerClient->isOK())
				std::cerr<<"SUBSERVER-CLIENT IS NOT WOIRKING CORRECTLY!"<<std::endl;
			else {
				int sec = -1;
				int usec=-1;
				if(timeout>0){
					sec = (int)timeout;
					usec = (int)((timeout-sec)*1000);
				}
				switch ( selret = selectable::run(sec,usec) ) {
					case -1:
					//				if ( errno == EINTR ) continue;
					eperror("select() call failed");
					//				wantexit++;
					break;
					case 0:
						// timeout (does not occour)
						break;
					default:
						subServerClient->recvpacket(tmpPacket);
						receivedPackets.push(tmpPacket);
				}//end switch
				//look if there are new packages and extract data
				while(this->getReceivedPacket(tmpPacket)){
					extractData(tmpPacket);
				}
				this->repeatedActions();
			}
			packetData_t data;
			while(getData(data)){
				analyseData(data);
			}
		}
	isKilled=true;
	cout<<"killed Client"<<endl;
}

void subClientHandler::getAboFromServer()
{

	int selret=0;
	if (this->subServerClient!=NULL){
		if(this->subServerClient->isOK()){
			switch ( selret = selectable::run() ) {
			case -1:
//				if ( errno == EINTR ) continue;
				eperror("select() call failed");
//				wantexit++;
				break;
			case 0:
				// timeout (does not occour)
				break;
			default:
				packet_t tmpPacket;
				subServerClient->recvpacket(tmpPacket);
				receivedPackets.push(tmpPacket);
				tmpPacket=packet_t();
				while(this->getReceivedPacket(tmpPacket)){
					extractData(tmpPacket);
				}
		}
		}
		else
			std::cerr<<"SUBSERVER-client is not working correctly"<<std::endl;
	}
	else
		std::cerr<<"SUBSERVER-client is not working correctly"<<std::endl;
}



void* subClientHandler::receiveAbos(void*)
{
		vector<string>abos;
		if(!subServerClient->isOK())
			std::cerr<<"SUBSERVER-CLIENT IS NOT WOIRKING CORRECTLY!"<<std::endl;

		while(!bKillClient){
			if(!subServerClient->isOK())
				std::cerr<<"SUBSERVER-CLIENT IS NOT WOIRKING CORRECTLY!"<<std::endl;
			else {

				//std::cout<<"get abo from server"<<std::endl;
				this->getAboFromServer();
				//std::cout<<"get package"<<std::endl;
				packet_t tmpPacket;
				while(this->getReceivedPacket(tmpPacket)){
					extractData(tmpPacket);
				}

			}
			packetData_t data;
			while(getData(data)){
				analyseData(data);
			}
		}
		isKilled=true;
		cout<<"killed Client"<<endl;
		return NULL;
}



bool subClientHandler::getReceivedPacket(packet_t &receivedPacket)
{
	if(receivedPackets.empty())
		return false;
	else {
		packet_t output = receivedPackets.front();
		receivedPackets.pop();
		receivedPacket=output;
		return true;
	}
}

void subClientHandler::setClientName(std::string clientName)
{
    this->clientName=clientName;
	subServerClient->setid(clientName.c_str());
	std::cout<<"SubserverClientID is set to:\""<<clientName.c_str()<<"\""<<std::endl;
}

void subClientHandler::saveData(packetData_t data)
{
	this->dataQueue.push(data);
}

/**
 * extract DAta out of packet and saves same to dataQueue
 * @param packet oacket where data should be extracted
 */
bool subClientHandler::extractData(packet_t packet)
{
	//std::cout<<"Abo: "<<packet.name()<<"***************analysing Packet: "<<packet.data()<<std::endl;
	std::string packetContent=packet.data();
	size_t found=packetContent.find_first_of(" ");
	if(found!=packetContent.size()) {
		packetData_t newPacket;
		newPacket.timeStamp = atoi(packetContent.substr(0,found).c_str());
		newPacket.data = packetContent.substr(found+1,packetContent.length()-found-2);
		newPacket.aboName = packet.name();
		//std::cout<<"  AboName: \""<<newPacket.aboName<<"\""<<std::endl;
		//std::cout<<"TimeStamp: \""<<newPacket.timeStamp<<"\""<<std::endl;
		//std::cout<<"     Data: \""<<newPacket.data<<"\""<<std::endl;
        string data = newPacket.data;
        std::transform(data.begin(), data.end(), data.begin(), ::tolower);
        if(data.find("ping")!=std::string::npos){
            echoToPing(newPacket.aboName);
            return false;
        }
		saveData(newPacket);
		return true;
	}
	else
		return false;
}

void subClientHandler::sendError(std::string errorMessage){
	std::string aboName ="/errors";
	this->sendToServer(aboName,errorMessage);
}

void subClientHandler::setSubscribedAbos(std::vector<std::string> vecAbos){
	for(unsigned int i=0;i<vecAbos.size();i++){
		this->subscribeAbo(vecAbos.at(i));

	}
}
void subClientHandler::subscribeAbo(std::string aboName)
{
	if(this->subscribedAbos.find(aboName)==this->subscribedAbos.end()){
		subServerClient->subscribe(aboName.c_str());
		this->subscribedAbos.insert(aboName);
		std::cout<<"subscribed Abo \""<<aboName.c_str()<<"\""<<std::endl;
		//this->sendToServer(aboName,"Ich habe dieses Abo Aboniert...");
	}
}

void subClientHandler::supplyAbo(std::string aboName){
	if(this->suppliedAbos.find(aboName)==this->suppliedAbos.end()){
		this->subServerClient->supply(aboName.c_str());
		std::cout<<"supplying abo:\""<<aboName<<"\""<<std::endl;
		this->suppliedAbos.insert(aboName);
	}
}
void subClientHandler::unsupplyAbo(std::string aboName){
	if(this->suppliedAbos.find(aboName)!=this->suppliedAbos.end()){
		this->subServerClient->unsupply(aboName.c_str());
		this->suppliedAbos.erase(aboName);
	}
}

void subClientHandler::unsubscribeAll(){
	while(!this->subscribedAbos.empty()){
		unsubscribeAbo(*this->subscribedAbos.begin());
	}
}

void subClientHandler::unsubscribeAbo(std::string aboName){
	if(this->subscribedAbos.find(aboName)!=subscribedAbos.end()){
		this->subServerClient->unsubscribe(aboName.c_str());
		this->subscribedAbos.erase(aboName);
	}
}
void subClientHandler::unsupplyAll(){
	while(!this->suppliedAbos.empty()){
		unsupplyAbo(* this->suppliedAbos.begin());
	}

}

void subClientHandler::closeConnection(){
	unsupplyAll();
	unsubscribeAll();
	subServerClient->terminate();

}
