
#include <stdio.h>
#include <errno.h>
#include <unistd.h>	// read
#include <stdlib.h>	// atexit

#ifdef _WIN32
	#include <winsock2.h>
#else
	#include <sys/socket.h>	// socket
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>	// inet_aton
#endif

#include <string.h>

#include "error.h"

#include "clientinfo.h"
#include "abo.h"
#include "packet_t.h"
#include "subserver.h"
#include "daemon.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//using namespace std;

int GlobalLogLevel = 0;

// PID_FILE must be absolute path, because server calls chdir()
#define PID_FILE	"/var/tmp/subserver.pid"

// stdio redirection for daemonize_me()
#define ALT_STDOUT_FILE	"server.stdout"
#define ALT_STDERR_FILE "server.stderr"
#define ALT_STDIN_FILE	"/dev/null"
int redirect_stdio() {
	int fd;
	// STDIN
	fd = open(ALT_STDIN_FILE, O_RDONLY | O_CREAT | O_NOCTTY);
	if (fd < 0) {
		eperror("could not open alternative stdin '%s'", ALT_STDIN_FILE);
		return(1);
	}
	close(STDIN_FILENO);	// close current std*
	if ( dup2(fd, STDIN_FILENO) < 0 ) {	// "stdin = fd";
		eperror("could not dup2() stdin '%s'", ALT_STDIN_FILE);
		return(1);
	}
	close(fd);		// close help fd;

	// STDOUT
	fd = open(ALT_STDOUT_FILE, O_WRONLY | O_APPEND | O_CREAT | O_NOCTTY, S_IRUSR|S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		eperror("could not open alternative stdout '%s'", ALT_STDOUT_FILE);
		return(1);
	}
	close(STDOUT_FILENO);	// close current std*
	if ( dup2(fd, STDOUT_FILENO) < 0 ) {	// "stdin = fd";
		eperror("could not dup2() stdout '%s'", ALT_STDOUT_FILE);
		return(1);
	}
	close(fd);		// close help fd;
	
	// STDERR
	fd = open(ALT_STDERR_FILE, O_WRONLY | O_APPEND | O_CREAT | O_NOCTTY, S_IRUSR|S_IWUSR | S_IRGRP | S_IROTH );
	if (fd < 0) {
		eperror("could not open alternative stderr '%s'", ALT_STDERR_FILE);
		return(1);
	}
	close(STDERR_FILENO);	// close current std*
	if ( dup2(fd, STDERR_FILENO) < 0 ) {	// "stdin = fd";
		eperror("could not dup2() stderr '%s'", ALT_STDERR_FILE);
		return(1);
	}
	close(fd);		// close help fd;
	
	return 0;
}

#define MAX_LINELEN	256
int run_configfile(char* filename, subserver* me) {
	FILE* fil;
	packet_t packet;
	packet.type = PKT_MANAGEMENT;
	packet.setName("/configfile");
	eprintf("config: reading file %s\n", filename);
	if ( ( fil=fopen(filename, "r") ) == NULL ) {
		eperror("could not open config file \"%s\"", filename);
		return(-1);
	}

	char line[MAX_LINELEN] = {0};
	while ( ! (feof(fil) | ferror(fil)) ) {
		if ( fgets(line, MAX_LINELEN, fil) == NULL ) break;
		packet.setData(line, strlen(line));
		eprintf("config> %s", line);
		me->do_mgm_packet(&packet);
	}

	eprintf("config: EOF %s\n", filename);
	fclose(fil);
	return 0;
}

void clean_exit() {
	if ( unlink(PID_FILE) < 0 ) {
		eperror("could not remove pid file");
	}
	eprintf("*** server terminated ***\n");
	exit(0);
}

void sighand(int s) {
	eprintf("signal(%d) %s received: going down now!\n", s, SIGNAME[s]);
	clean_exit();
}

void printhelp() {
	eprintf("\n\
USAGE:	server [OPTIONS]\n\
OPTIONS are:\n\
	-c <file>,\n\
	--config <file>\n\
		Read a config file, as if each line is sent to the server\n\
		as a PKT_MANAGEMENT packet.\n\
	--nodaemon\n\
		Do not change the subserver to a background process. Using\n\
		this flag prevents the i/o redirection and all output will\n\
		be sent to the normal stderr and stdout.\n\
	-h, --help\n\
		Print this help screen.\n\
\n\
");
}

int main(int argc, char* argv[]) {

	// BEGIN PARSE COMMANDLINE
	int daemon=1;
	char* configfile = NULL;
	char oldworkdir[1024];
	getcwd(oldworkdir,1024);// = get_current_dir_name();

	for (int i=1; i<argc; i++) {
		if        ( strcmp(argv[i], "--nodaemon") == 0 ) {
			daemon = 0;
		} else if ( strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0 ) {
			if ( i+1 < argc ) {
				configfile = argv[i+1];
				i++;
			} else {
				eprintf("no config file given to --config option!\n");
				exit(1);
			}
		} else if ( strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 ) {
			printhelp();
			exit(0);
		} else {
			eprintf("unknown command line option: %s\n", argv[i]);
			eprintf("use --help to see valid options!\n");
			exit(1);
		}
	}
	// END   PARSE COMMANDLINE

	// daemonize and signal handler setup
	if ( daemon ) {
		int fd = open(PID_FILE, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if ( fd < 0 ) {
			eperror("could not open pid file %s", PID_FILE);
			eprintf("server exiting\n");
			exit(1);
		}
		if ( redirect_stdio() != 0 ) { exit(1); };
		daemonize_me();
		char buffer[32];
		sprintf(buffer, "%d", getpid());
		write(fd, buffer, strlen(buffer));
		close(fd);
		catch_signals(sighand);
		eprintf("\n*** server started as pid %d ***\n", getpid());
	} else {	
		eprintf("server starting...\n");
		chdir("/");	// this is done by daemonize_me() and if not done manually here
				// causes log directories to be created from/in current workdir
	}
	atexit(clean_exit);

	// create subserver instance
	subserver me;

	if ( !me.isOK() ) {
		eprintf("server did not start successfully. Terminating.\n");
		exit(1);
	}

	// read config file
	if (configfile != NULL) {
//		chdir(oldworkdir);	// FIXME
		if ( run_configfile(configfile, &me) < 0) {	// this opens logfile in wrong dir!
			eprintf("problems reading config file!\n");
			exit(1);
		}
//		chdir("/");	// maybe this should be some kind like LOGPATH
	}
	clientinfo*	client;
	clientinfo*	tgt;
	abo*		A;
	packet_t	packet;
	int		txlen;

	//
	// M A I N   L O O P
	//
	while (1) {
		
		if ( me.recvpacket(packet) < 0 ) {
			// received error
			continue;
		}
		client = clientinfo::Client(&(packet.address));

		if ( VERBOSE && SHOW_PACKETS ) {
			eprintf("rx ");
			packet.print();
		}

		A = NULL;
		switch (packet.type) {
		case PKT_MANAGEMENT:
			// subserver::parse(packet.data(), packet.datalen(), argc, argv);
			me.do_mgm_packet(&packet);
			break;
		case PKT_SETDATA:
		case PKT_DATA:
			A = abo::Abo(packet.name(), packet.namelen());
			A->received();
			// prepare abo packet
			if ( A->getFlags() & AF_ADDTIME ) {		// AF_ADDTIME
				char buffer[MAX_PACKETLEN];
				int len=snprintf(buffer, MAX_PACKETLEN-2, "%ld\t", time(NULL));
				int remain = MAX_PACKETLEN - len;
				if ( remain > packet.datalen() ) remain = packet.datalen();
				memcpy(&(buffer[len]), packet.data(), remain);
				len += remain;
				packet.setData(buffer, len);
			}
			if ( A->getLogLevel() > GlobalLogLevel ) {	// LogLevel (FIXME: AF_LOG)
				if (A->logPacket(packet) < 0 ) {
					eprintf("%s:%d: failed to log data!\n", __FILE__,__LINE__);
				}
			}
			if ( packet.type == PKT_SETDATA ) {
				for (int i = 0; i<A->Suppliers(); i++) {
					tgt = A->supplier[i];
					if ( tgt == client ) continue; // don't send to source client
					packet.setAddress(tgt->address, sizeof(tgt->address));
					me.sendpacket(packet);
				}
			} else { // PKT_DATA
				for (int i = 0; i<A->Subscribers(); i++) {
					tgt = A->subscriber[i];
					if ( tgt == client ) continue; // don't send to source client
					packet.setAddress(tgt->address, sizeof(tgt->address));
					me.sendpacket(packet);
				}
			}
			//break; // don't break for automatic supply registration
		case PKT_SUPPLY:
			if (A == NULL) A = abo::Abo(packet.name(), packet.namelen());
			if ( client->addSupply(A) > 0 )
				eprintf("client %s now supplies %s abo\n", client->addrString(), A->name);
			if ( A->addSupplier(client) > 0 )
				eprintf("abo %s is now supplied by %s\n", A->name, client->addrString());
			break;
		case PKT_UNSUPPLY:
			if (A == NULL) A = abo::Abo(packet.name(), packet.namelen());
			if ( client->removeSupply(A) > 0 )
				eprintf("client %s no longer supplies %s abo\n", client->addrString(), A->name);
			if ( A->removeSupplier(client) > 0 )
				eprintf("abo %s is no longer supplied by %s\n", A->name, client->addrString());
			break;
		case PKT_SUBSCRIBE:
			if (A == NULL) A = abo::Abo(packet.name(), packet.namelen());
			if ( client->addSubscription(A) > 0 ) 
				eprintf("client %s now subscribed %s\n", client->addrString(), A->name);
			if ( A->addSubscriber(client) > 0 )
				eprintf("abo %s is now subscribed by %s\n", A->name, client->addrString());
			break;
		case PKT_UNSUBSCRIBE:
			if (A == NULL) A = abo::Abo(packet.name(), packet.namelen());
			if ( client->removeSubscription(A) > 0 )
				eprintf("client %s no longer subscribes %s abo\n", client->addrString(), A->name);
			if ( A->removeSubscriber(client) > 0 )
				eprintf("abo %s is no longer subscribed by %s\n", A->name, client->addrString());
			break;
		case PKT_CLIENTTERM:
			eprintf("del client: %s\n", client->addrString());
			delete client;
			client=NULL;
			break;
		default:
			eprintf("WARNING: packet has invalid type: ");
			packet.print();
		}
	} // end main loop

	eprintf("server going down...\n");
	return 0;
} // end main()

