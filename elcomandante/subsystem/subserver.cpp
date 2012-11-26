/*
 * subserver.cpp
 * server class
 * Dennis Terhorst
 * 14 Mar 2008
 */
#include "subserver.h"
#include <unistd.h>	// close

/*
struct sockaddr_in {
	sa_family_t    sin_family; // address family: AF_INET
	u_int16_t      sin_port;   // port in network byte order
	struct in_addr  sin_addr;  // internet address
};

// Internet address.
struct in_addr {
	u_int32_t      s_addr;     // address in network byte order
};

*/

// getsockopt(IP_MTU) // see man ip(7)


// get size of next message in bytes or zero if no dgm pending
//int value;
//if ( (error=ioctl(sfd, SIOCINQ, &value)) != 0 ) {
//	eperror("ERROR: ioctl(SIOCINQ) failed");
//	return 0;
//}

// get size of local send queue in bytes
//int value;
//if ( (error=ioctl(sfd, SIOCOUTQ, &value)) != 0 ) {
//	eperror("ERROR: ioctl(SIOCOUTQ) failed");
//	return 0;
//}


//
// constructor's helper
//
int subserver::init(const char* serverip, const short serverport) {
	//
	// create network connection
	// 
	eprintf("  creating socket...\n");
	sfd = socket(PF_INET, SOCK_DGRAM, 0);
	if ( sfd < 0) { eperror("ERROR: socket() call returned %d", sfd); return -1; }

	eprintf("  building struct sockaddr_in...\n");
	struct sockaddr_in myaddr;//, src_addr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(serverport);
	inet_aton(serverip, &(myaddr.sin_addr));

	int binderr = bind(sfd, (struct sockaddr*)&myaddr, sizeof(myaddr));
	if (binderr != 0) { eperror("ERROR: bind() returned %d:",binderr); terminate(); return -1; }
	eprintf("  successfully bound to address %s:%d\n", inet_ntoa(myaddr.sin_addr), ntohs(myaddr.sin_port));

	//
	// init mgm command interface
	//
	init_commands();

	return 0;
}


subserver::subserver() {
	//
	// select ip:port by ourself
	//
	sfd=-1;
	char buffer[ENVBUFLEN];
	char *svr = getenv(SUBSERVER_ENVNAME);
	char *port_ptr;
	int port;
	int envfound=0;
	if (svr != NULL) {	// server from environment
		strncpy(buffer, svr, ENVBUFLEN-1);
		buffer[ENVBUFLEN-1] = 0;
		if ( (port_ptr = strchr(svr,':') ) != NULL) {
			*port_ptr=0;
			port_ptr++;
			port = atoi(port_ptr);
			if (port<65536 && port > 0) {
				init(svr, port);
				envfound=1;
			}
		}
	}
	if (! envfound) {	// use default compiled in server
		init(SUBSERVER_DEFAULT_ADDR,SUBSERVER_DEFAULT_PORT);
	}

	myCPUinfo = new CPU_info();
}

subserver::subserver(const char* serverip, const int serverport) {
	//
	// listen on given ip:port
	//
	sfd=-1;
	init(serverip, serverport);
}


subserver::~subserver() {
	terminate();
}


int subserver::getfd() const {
	return sfd;
}


bool subserver::isOK() const {
	return (sfd>0);
}


int subserver::sendpacket(packet_t& packet)	// cannot be const because of htons()
{
	int txlen=0;
	if ( VERBOSE & SHOW_PACKETS ) {
		eprintf("tx ");
		packet.print();
	}
	if ( packet.address.sin_addr.s_addr == 0 ) {
		if ( VERBOSE & SHOW_PACKET_DROPS ) {
			eprintf("drop ");
			packet.print();
		}
		eprintf("0.0.0.0: %s", packet.data());
		return 0;
	}
	// packet is always sent in network byteorder
	packet.type = htons(packet.type);
	packet.namelength = htons(packet.namelength);
	if ((txlen=sendto(sfd, packet.raw, packet.length, 0, (struct sockaddr*)&(packet.address), sizeof(packet.address))) < 0) {
		eperror("ERROR: local send() error");
	}
	// packet convert bytes back to keep packet constant
	packet.type = ntohs(packet.type);
	packet.namelength = ntohs(packet.namelength);
	if ( txlen < packet.length ) {
		eprintf("WARNING: truncated packet(len=%d) to %d bytes.\n", packet.length, txlen);
	}
	return txlen;
}


int subserver::recvpacket(packet_t& packet) {
	packet.addrlen=sizeof(packet.address);
	errno=0;
	packet.length = recvfrom(sfd,
				 packet.raw,
				 packet_t::MAX_LENGTH,
				 0,
				 (struct sockaddr*)&(packet.address), &(packet.addrlen)
			);
	// packet is always stored locally in host byteorder
	packet.type = ntohs(packet.type);
	packet.namelength = ntohs(packet.namelength);
				
	if (errno != 0 || packet.length < 0 ) {
		eperror("recvfrom() returned %d", packet.length);
		return -1;
	}

	clientinfo* client;
	client = clientinfo::Client(&(packet.address));
	//if ( client->lastrecv == time(NULL) ) {
	//	eprintf("WARNING: high packet rate from %s\n", client->addrString());
	//}
	client->received();		// receive hook (timer)

	if (! packet.isValid() ) {
		eprintf("WARNING: Discarding invalid packet from %s!", packet.addrString());
		return -2;
	}

	return 0;
}


void subserver::terminate() {
	if (sfd>=0) close(sfd);
	sfd=-1;
}

void subserver::do_mgm_packet(packet_t* packet)
{
	if (packet->type != PKT_MANAGEMENT ) {
		eprintf("%s:%d Packet type not PKT_MANAGEMENT!\n", __FILE__, __LINE__);
		return;
	}

	char *argv[16];
	int argc=0;
	clientinfo* client;
	// gather info
	client = clientinfo::Client(&(packet->address));
	command_t<SUBSERVER_CMDFUNC>::parse(packet->data(), packet->datalen(), argc, argv);	// FIXME: this should not be template dependant

	bool found = false;
	if (argc>0) {
		for (int i=0; i<cmd.length(); i++) {
			if   ( strcmp(argv[0], cmd[i].name) == 0 )
			{
				(this->*cmd[i].func)(client, argc, argv);
				found=true;
				break;
			}	
		}
	}
	if ( !found ) {
		unknowncommand(client, argc, argv);
	}

	// return
	return;
}


//
// SUBSERVER MANAGEMENT FUNCTIONS
//
int subserver::unknowncommand(clientinfo* client, int argc, char* argv[]) {
	senderror(client, "ERROR: unknown command!\n");
	for (int i=0; i<argc; i++) {			// print
		senderror(client, " arg[%d]: '%s'\n", i, argv[i]);
	}
	return 0;
}


int subserver::senderror(clientinfo* client, const char* format, ...) {
	packet_t packet;
	char buffer[MAX_PACKETLEN-30];	// FIXME: arbitrary boundary
	va_list va;
	va_start(va, format);
	int n=vsnprintf(buffer, MAX_PACKETLEN-30, format, va);
	va_end(va);
	
	packet.setAddress(client->address, sizeof(client->address));	// give info to requesting client
	packet.setName("/server/commanderror");
	packet.pprintf(buffer);
	sendpacket(packet);
	return n;
}


void subserver::init_commands() {
	cmd.add( command_t<SUBSERVER_CMDFUNC>("ls",		&subserver::cmd_listclients)	);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("la",		&subserver::cmd_listabos)	);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("lc",		&subserver::cmd_listclients)	);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("lg",		&subserver::cmd_listgraphical)	);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("setid",	&subserver::cmd_setClientID)	);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("log",	&subserver::cmd_log)		);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("nolog",	&subserver::cmd_nolog)		);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("time",	&subserver::cmd_time)		);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("notime",	&subserver::cmd_notime)		);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("unique",	&subserver::cmd_unique)		);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("nounique",	&subserver::cmd_nounique)		);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("rma",	&subserver::cmd_removeabos)		);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("rmc",	&subserver::cmd_removeclients)		);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("?",	&subserver::cmd_help)		);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("h",	&subserver::cmd_help)		);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("help",	&subserver::cmd_help)		);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("restart",&subserver::cmd_restart)	);
	cmd.add( command_t<SUBSERVER_CMDFUNC>("health",&subserver::cmd_health)	);	// get the health status of the server
	return;
}


int subserver::cmd_listclients(clientinfo* client, int argc, char* argv[]) {
	packet_t packet;

	packet.setAddress(client->address, sizeof(client->address));	// give info to requesting client
	packet.setName("/server/clientlist");
	packet.pprintf("%d client%s%s\n",
		clientinfo::noclientinfos,
		(clientinfo::noclientinfos==1?"":"s"),
		(clientinfo::noclientinfos>0?":":""));
	sendpacket(packet);
	for (int i=0; i<clientinfo::noclientinfos; i++) {
		packet.pprintf("%-30s %-21s %3d %3d +%ld\n",
			clientinfo::list[i]->id,
			clientinfo::list[i]->addrString(),
			clientinfo::list[i]->Supplies(),
			clientinfo::list[i]->Subscriptions(),
			time(NULL)-clientinfo::list[i]->lastrecv);
		sendpacket(packet);

		if (argc > 1 && strcmp(argv[1],"-l")==0 ) {
			for (int a=0; a<clientinfo::list[i]->Supplies(); a++) {
				packet.pprintf("     supplies   %s\n", 
					clientinfo::list[i]->supply[a]->name);
				sendpacket(packet);
			}
			for (int a=0; a<clientinfo::list[i]->Subscriptions(); a++) {
				packet.pprintf("     subscribed %s\n",
					clientinfo::list[i]->subscription[a]->name);
				sendpacket(packet);
			}
		}
	}
	return 0;
}


int subserver::cmd_listgraphical(clientinfo* client, int argc, char* argv[]) {
	packet_t packet;
	const char color_red[100] = "color=red, ";

	packet.setAddress(client->address, sizeof(client->address));	// give info to requesting client
	packet.setName("/server/clientlist_graph");

	packet.pprintf("digraph subservices {\n"); sendpacket(packet);

	// make list of available abos
	packet.pprintf("\n\t// list of abos\n"); sendpacket(packet);
	packet.pprintf("\tnode [shape=ellipse, color=cornsilk3, style=filled];\n"); sendpacket(packet);
	for (int i=0; i<abo::noabos; i++) {
		packet.pprintf("\tabo%03d [%slabel=<%s<BR/><FONT POINT-SIZE=\"10\">%c%c%c %d/%d +%ld</FONT>>];\n",
			i,
			(time(NULL)-abo::list[i]->lastrecv > 60 ? color_red : ""),
			abo::list[i]->name,
			(abo::list[i]->getLogLevel()>0?'L':'-'),
			((abo::list[i]->getFlags()&AF_ADDTIME)>0?'T':'-'),
			((abo::list[i]->getFlags()&AF_UNIQSUPPLY)>0?'U':'-'),
			abo::list[i]->Suppliers(),
			abo::list[i]->Subscribers(),
			time(NULL)-abo::list[i]->lastrecv);
		sendpacket(packet);
	}
	// make list of clients and link their subscriptions and supplies
	packet.pprintf("\n\t// list of clients and connections\n"); sendpacket(packet);
	packet.pprintf("\tnode [ shape=box, color=dodgerblue2, style=solid ];\n"); sendpacket(packet);
	packet.pprintf("\tedge [len=2];   // stretch the drawing a bit for neato\n"); sendpacket(packet);

	for (int i=0; i<clientinfo::noclientinfos; i++) {
		packet.pprintf("\n\tclient%03d [label=<%s<BR/><FONT POINT-SIZE=\"10\">%s</FONT>>]\n",
			i, 
			clientinfo::list[i]->id,
			clientinfo::list[i]->addrString());
		sendpacket(packet);
	
		// supplies
		for (int a=0; a<clientinfo::list[i]->Supplies(); a++) {
			packet.pprintf("\t\tclient%03d -> abo%03d\n", i, clientinfo::list[i]->supply[a]->index());
			sendpacket(packet);
//			packet.pprintf("     supplies   %s\n", 
//				clientinfo::list[i]->supply[a]->name);
//			sendpacket(packet);
		}
		// subscriptions
		for (int a=0; a<clientinfo::list[i]->Subscriptions(); a++) {
			packet.pprintf("\t\tabo%03d -> client%03d\n", clientinfo::list[i]->subscription[a]->index(), i);
			sendpacket(packet);
//			packet.pprintf("     subscribed %s\n",
//				clientinfo::list[i]->subscription[a]->name);
//			sendpacket(packet);
		}
	
	}
	// OLD FUNCTION
	/*
	for (int i=0; i<clientinfo::noclientinfos; i++) {
		packet.pprintf("\tclient%03d [label=<%s<BR/><FONT POINT-SIZE=\"10\">%s</FONT>>]\n",
			i, 
			clientinfo::list[i]->id,
			clientinfo::list[i]->addrString());
		sendpacket(packet);
	}

	for (int i=0; i<abo::noabos; i++) {
		packet.pprintf("\tabo%03d [label=<%s<BR/><FONT POINT-SIZE=\"10\">%c%c%c %d/%d +%ld</FONT>>];\n",
			i,
			abo::list[i]->name,
			(abo::list[i]->getLogLevel()>0?'L':'-'),
			((abo::list[i]->getFlags()&AF_ADDTIME)>0?'T':'-'),
			((abo::list[i]->getFlags()&AF_UNIQSUPPLY)>0?'U':'-'),
			abo::list[i]->Suppliers(),
			abo::list[i]->Subscribers(),
			time(NULL)-abo::list[i]->lastrecv);
		sendpacket(packet);
	}

	// client sorted links
	packet.pprintf("\n\tedge [len=2];   // stretch the drawing a bit for neato\n"); sendpacket(packet);
	for (int c=0; c<clientinfo::noclientinfos; c++) {
		packet.pprintf("\n\t// client%03d %s\n",c, clientinfo::list[c]->id); sendpacket(packet);
		for (int a=0; a<clientinfo::list[c]->Supplies(); a++) {
			packet.pprintf("\tclient%03d -> abo%03d\n", c, clientinfo::list[c]->supply[a]->index());
			sendpacket(packet);
		}
		for (int a=0; a<clientinfo::list[c]->Subscriptions(); a++) {
			packet.pprintf("\tabo%03d -> client%03d\n", clientinfo::list[c]->supply[a]->index(), c);
			sendpacket(packet);
		}
	}*/

	packet.pprintf("}\n"); sendpacket(packet);
	return 0;
}


int subserver::cmd_removeclients(clientinfo* client, int argc, char* argv[]) {
	if (argc != 2) {
		senderror(client, "%s must have exactly one parameter!\n", argv[0]);
		return -1;
	}
	packet_t packet;

	packet.setAddress(client->address, sizeof(client->address));	// give info to requesting client
	packet.setName("/server/clientlist");
	for (int i=0; i<clientinfo::noclientinfos; i++) {
		//packet.pprintf("%-30s %-21s %3d %3d +%ld\n",
		//	clientinfo::list[i]->id,
		//	clientinfo::list[i]->addrString(),
		//	clientinfo::list[i]->Supplies(),
		//	clientinfo::list[i]->Subscriptions(),
		//	time(NULL)-clientinfo::list[i]->lastrecv);
		//sendpacket(packet);

		if (argc > 1 &&
		            ( strcmp(argv[1],clientinfo::list[i]->id)==0 
			    ||strcmp(argv[1],clientinfo::list[i]->addrString()) == 0
			    )	   ) {
			packet.pprintf("deleting %-30s %-21s %3d %3d +%ld\n",
				clientinfo::list[i]->id,
				clientinfo::list[i]->addrString(),
				clientinfo::list[i]->Supplies(),
				clientinfo::list[i]->Subscriptions(),
				time(NULL)-clientinfo::list[i]->lastrecv);
			sendpacket(packet);
		   	delete(clientinfo::list[i]);
		}
	}
	packet.pprintf("%d client%s left\n",
		clientinfo::noclientinfos,
		(clientinfo::noclientinfos==1?"":"s"));
	sendpacket(packet);
	return 0;
}

int subserver::cmd_listabos(clientinfo* client, int argc, char* argv[]) {
	packet_t packet;

	packet.setAddress(client->address, sizeof(client->address));	// give info to requesting client
	packet.setName("/server/abolist");
	packet.pprintf("%d current submission%s%s\n",
		abo::noabos, 
		(abo::noabos==1?"":"s"),
		(abo::noabos>0?":":""));
	sendpacket(packet);
	for (int i=0; i<abo::noabos; i++) {
		if(abo::list[i]->lastrecv != 0){
			packet.pprintf("%c%c%c %-30s %3d %3d +%ld\n",
				(abo::list[i]->getLogLevel()>0?'l':'-'),
				((abo::list[i]->getFlags()&AF_ADDTIME)>0?'t':'-'),
				((abo::list[i]->getFlags()&AF_UNIQSUPPLY)>0?'U':'-'),
				abo::list[i]->name,
				abo::list[i]->Suppliers(),
				abo::list[i]->Subscribers(),
				time(NULL)-abo::list[i]->lastrecv);
		}else{
			packet.pprintf("%c%c%c %-30s %3d %3d unsupplied\n",
				(abo::list[i]->getLogLevel()>0?'l':'-'),
				((abo::list[i]->getFlags()&AF_ADDTIME)>0?'t':'-'),
				((abo::list[i]->getFlags()&AF_UNIQSUPPLY)>0?'U':'-'),
				abo::list[i]->name,
				abo::list[i]->Suppliers(),
				abo::list[i]->Subscribers());
		}
		sendpacket(packet);
		if (argc > 1 && strcmp(argv[1],"-l")==0 ) {
			for (int a=0; a<abo::list[i]->Suppliers(); a++) {
				packet.pprintf("     supplied by   %-25s(%s)\n",
					abo::list[i]->supplier[a]->id,
					abo::list[i]->supplier[a]->addrString());
				sendpacket(packet);
			}
			for (int a=0; a<abo::list[i]->Subscribers(); a++) {
				packet.pprintf("     subscribed by %-25s(%s)\n",
					abo::list[i]->subscriber[a]->id,
					abo::list[i]->subscriber[a]->addrString());
				sendpacket(packet);
			}
		}
	}
	return 0;
}

int subserver::cmd_removeabos(clientinfo* client, int argc, char* argv[]) {
	if (argc != 2) {
		senderror(client, "%s must have exactly one parameter!\n", argv[0]);
		return -1;
	}
	packet_t packet;

	packet.setAddress(client->address, sizeof(client->address));	// give info to requesting client
	packet.setName("/server/abolist");
	for (int i=0; i<abo::noabos; i++) {
		//packet.pprintf("%c %-30s %3d %3d +%ld\n",
		//	(abo::list[i]->getLogLevel()>0?'l':'-'),
		//	abo::list[i]->name,
		//	abo::list[i]->Suppliers(),
		//	abo::list[i]->Subscribers(),
		//	time(NULL)-abo::list[i]->lastrecv);
		//sendpacket(packet);
		if (argc > 1 && strcmp(argv[1],abo::list[i]->name)==0 ) {
			packet.pprintf("deleting %c %-30s %3d %3d +%ld\n",
				(abo::list[i]->getLogLevel()>0?'l':'-'),
				abo::list[i]->name,
				abo::list[i]->Suppliers(),
				abo::list[i]->Subscribers(),
				time(NULL)-abo::list[i]->lastrecv);
			sendpacket(packet);
		
			delete(abo::list[i]);
		}
		if (argc > 1 && strcmp(argv[1],"unsupplied")==0 ) {
			if ( abo::list[i]->Suppliers() == 0 ) {
				packet.pprintf("deleting %c %-30s %3d %3d +%ld\n",
					(abo::list[i]->getLogLevel()>0?'l':'-'),
					abo::list[i]->name,
					abo::list[i]->Suppliers(),
					abo::list[i]->Subscribers(),
					time(NULL)-abo::list[i]->lastrecv);
				sendpacket(packet);
			
				delete(abo::list[i]);
			}
		}
	}
	packet.pprintf("%d submission%s left.\n",
		abo::noabos, 
		(abo::noabos==1?"":"s"));
	sendpacket(packet);
	return 0;
}


int subserver::cmd_setClientID(clientinfo* client, int argc, char* argv[]) {
	if (argc != 2) {
		senderror(client, "%s must have exactly one parameter!\n", argv[0]);
		return -1;
	}
	client->setID(argv[1]);
	eprintf("client %s announced to be %s\n", client->addrString(), client->id );
	return 0;
}


int subserver::cmd_nolog(clientinfo* client, int argc, char* argv[]) {
	if (argc != 2) {
		senderror(client, "%s must have exactly one parameter!\n", argv[0]);
		return -1;
	}
	char* aboname = argv[1];

	abo* A = abo::Abo(aboname, strlen(aboname));
	A->setLogLevel(A->getLogLevel()-1);
	packet_t packet;
	packet.setAddress(client->address, sizeof(client->address));
	packet.setName("/server/logging");
	packet.pprintf("%s loglevel %d\n", aboname, A->getLogLevel());
	sendpacket(packet);
	return 0;
}


int subserver::cmd_log(clientinfo* client, int argc, char* argv[]) {
	if (argc != 2) {
		senderror(client, "%s must have exactly one parameter!\n", argv[0]);
		return -1;
	}
	char* aboname = argv[1];

	abo* A = abo::Abo(aboname, strlen(aboname));
	A->setLogLevel(A->getLogLevel()+1);
	packet_t packet;
	packet.setAddress(client->address, sizeof(client->address));
	packet.setName("/server/logging");
	packet.pprintf("%s loglevel %d\n", aboname, A->getLogLevel());
	sendpacket(packet);
	return 0;
}

int subserver::cmd_time(clientinfo* client, int argc, char* argv[]) {
	if (argc != 2) {
		senderror(client, "%s must have exactly one parameter!\n", argv[0]);
		return -1;
	}
	char* aboname = argv[1];

	abo* A = abo::Abo(aboname, strlen(aboname));
	packet_t packet;
	packet.setAddress(client->address, sizeof(client->address));
	packet.setName("/abo/flags");
	packet.pprintf("%s +AF_ADDTIME\n", aboname);
	A->setFlag(AF_ADDTIME);
	sendpacket(packet);
	return 0;
}
int subserver::cmd_notime(clientinfo* client, int argc, char* argv[]) {
	if (argc != 2) {
		senderror(client, "%s must have exactly one parameter!\n", argv[0]);
		return -1;
	}
	char* aboname = argv[1];

	abo* A = abo::Abo(aboname, strlen(aboname));
	packet_t packet;
	packet.setAddress(client->address, sizeof(client->address));
	packet.setName("/abo/flags");
	packet.pprintf("%s -AF_ADDTIME\n", aboname);
	A->delFlag(AF_ADDTIME);
	sendpacket(packet);
	return 0;
}
int subserver::cmd_unique(clientinfo* client, int argc, char* argv[]) {
	if (argc != 2) {
		senderror(client, "%s must have exactly one parameter!\n", argv[0]);
		return -1;
	}
	char* aboname = argv[1];

	abo* A = abo::Abo(aboname, strlen(aboname));
	packet_t packet;
	packet.setAddress(client->address, sizeof(client->address));
	packet.setName("/abo/flags");
	packet.pprintf("%s +AF_UNIQSUPPLY\n", aboname);
	A->setFlag(AF_UNIQSUPPLY);
	sendpacket(packet);
	return 0;
}
int subserver::cmd_nounique(clientinfo* client, int argc, char* argv[]) {
	if (argc != 2) {
		senderror(client, "%s must have exactly one parameter!\n", argv[0]);
		return -1;
	}
	char* aboname = argv[1];

	abo* A = abo::Abo(aboname, strlen(aboname));
	packet_t packet;
	packet.setAddress(client->address, sizeof(client->address));
	packet.setName("/abo/flags");
	packet.pprintf("%s -AF_UNIQSUPPLY\n", aboname);
	A->delFlag(AF_UNIQSUPPLY);
	sendpacket(packet);
	return 0;
}

int subserver::cmd_help(clientinfo* client, int argc, char* argv[]) {
	packet_t packet;
	packet.setAddress(client->address, sizeof(client->address));
	packet.setName("/server/help");

	packet.pprintf("***********************************************\n"); sendpacket(packet);
	packet.pprintf("SUBSERVER - by Dennis Terhorst\n"); sendpacket(packet);
	packet.pprintf("-----------------------------------------------\n"); sendpacket(packet);
	packet.pprintf("known commands in PKT_MANAGEMENT mode are:\n"); sendpacket(packet);
	packet.pprintf("\n"); sendpacket(packet);
	packet.pprintf("\thelp\t\tshow this help\n"); sendpacket(packet);
	packet.pprintf("\tsetid <name>\tset identifier <name> for current client\n"); sendpacket(packet);
	packet.pprintf("\tlc [-l]\t\tlist clients [and their subsciptions]\n"); sendpacket(packet);
	packet.pprintf("\tla [-l]\t\tlist abos [and their suppliers]\n"); sendpacket(packet);
	packet.pprintf("\trmc <name>\tremove client with id=<name> or addr=<name>\n"); sendpacket(packet);
	packet.pprintf("\trma <name>\tremove abo with name <name>\n"); sendpacket(packet);
	packet.pprintf("\trma unsupplied\tremove abos which are not supplied by any client\n"); sendpacket(packet);
	packet.pprintf("\t[no]log <name>\tdecrement or increment loglevel of abo <name>\n"); sendpacket(packet);
	packet.pprintf("\t[no]time <name>\tset/clear 'add time' flag of abo <name>\n"); sendpacket(packet);
	packet.pprintf("\t[no]unique <name>\tset/clear 'unique supply' flag of abo <name>\n"); sendpacket(packet);
	packet.pprintf("\n"); sendpacket(packet);
	packet.pprintf("***********************************************\n"); sendpacket(packet);
	packet.pprintf("\n"); sendpacket(packet);

	//packet.pprintf("\n"); sendpacket(packet);
	return 0;
}

int subserver::cmd_restart(clientinfo* client, int argc, char* argv[]) {
	cmd.clear();
	//terminate();
	init_commands();
	return 0;
}

int subserver::cmd_health(clientinfo* client, int argc, char* argv[]) {
	packet_t packet;
	packet.setAddress(client->address, sizeof(client->address));
	packet.setName("/server/health");

	float act, tot, load;

	packet.pprintf("server health status\n"); sendpacket(packet);

	myCPUinfo->getStats();	// GET CPU stat

	packet.pprintf("active CPU  : %f\n", myCPUinfo->actual()); sendpacket(packet);
	packet.pprintf("TOTAL CPU   : %f\n", myCPUinfo->total()); sendpacket(packet);
	packet.pprintf("Load Average: %f\n", myCPUinfo->average());  sendpacket(packet);

	packet.pprintf("\n"); sendpacket(packet);
	return 0;
}

