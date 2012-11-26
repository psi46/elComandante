#include <stdio.h>
#include <signal.h>
#include <unistd.h>

using namespace std;

#include <string.h> // strdup
#include <unistd.h> // fork
#include <stdlib.h> // free, exit
class child_t {
private:
	int PID;
	char* command;
public:
	child_t() { PID=-1; command=NULL; };

	child_t(const char* cmd) {
		PID=-1;
		command = strdup(cmd);
		if ( (PID=fork()) > 0) {	// parent
			printf("new child pid %d\n", PID);
		} else {
			printf("child here\n");
			sleep(5);
			printf("child exiting\n");
			exit(0);
		}
	}

	virtual ~child_t() {
		if (PID>0) {
			printf("terminating child(%d)\n", PID);
			kill(PID, SIGTERM);
		}
		free(command);	// free memory allocated with strdup
	}

	int pid() {
		return PID;
	}
};


#include <sys/types.h>
#include <wait.h>
#include <errno.h>
#include <string.h>
class kindergarden_t {
private:
	child_t *child[100];
	int noc;

	int set_sighandler() {
		printf("setting signal handler\n");
		struct sigaction sa;
		const int sig = SIGCHLD;
		printf("registering handler for signal %d\n", sig);
		sa.sa_handler = kindergarden_t::sigchld_handler;
		// see man sigsetops(3)
		sigemptyset(&(sa.sa_mask));
		sa.sa_flags = 0;

		if ( sigaction(sig, &sa, NULL) < 0 ) {
			fprintf(stderr, "could not register handler for signal %d: %s\n", sig, strerror(errno));
			return -1;
		}
		return 0;
	}

	static void sigchld_handler(int sig) {
		printf("kindergarden received SIGCHLD...\n");
		siginfo_t siginfo;

		siginfo.si_pid=0;
		if ( waitid(P_ALL, -1, &siginfo, WEXITED | WSTOPPED | WCONTINUED | WNOHANG) < 0 ) {
			perror("wait failed");
		}
		if ( siginfo.si_pid != 0 ) {
			printf("SIGCHLD: code=%s, pid=%d, uid=%d, return status=%d\n",
				siginfo.si_code==CLD_EXITED?"CLD_EXITED":
				siginfo.si_code==CLD_KILLED?"CLD_KILLED":
				siginfo.si_code==CLD_STOPPED?"CLD_STOPPED":
				siginfo.si_code==CLD_CONTINUED?"CLD_CONTINUED": "UNKNOWN",
				siginfo.si_pid, siginfo.si_uid, siginfo.si_status); // UNKNOWN should never appeear!
			if ( siginfo.si_code == CLD_KILLED || siginfo.si_code == CLD_EXITED ) {
				//children--;
			}
		}
	}
public:
	kindergarden_t() {
		printf("opening kindergarden\n");
		noc = 0;
		if ( this->set_sighandler() < 0) {
			perror("could not setup signal handler");
		}
	}

	virtual ~kindergarden_t() {
		for (int i=0; i<noc; i++) {
			delete child[i];
		}
		for (int i=0; i<5; i++) {
			if (0==noc) break;
			printf("waiting for %d children to terminate...\n", noc);
			sleep(1);
		}
		printf("closing kindergarden\n");
	}

	int add(child_t& neu) {
		child[noc] = &neu;
	}

	int NumberOfChildren() { return noc; }

	child_t* Child(pid_t pid) {
		for (int i=0; i<noc; i++) {
			if (child[i].pid() == pid) return child[i];
		}
		return NULL;
	}

};

int main(void) {
	printf("Hallo Welt\n");
	kindergarden_t kg;

	printf("kindergarden has %d children\n", kg.NumberOfChildren() );
	sleep(1);
	printf("raising sigCHLD\n");
	raise(SIGCHLD);
	printf("returned from raising sigCHLD\n");
	sleep(1);
	printf("exit\n");

	return 0;
}
