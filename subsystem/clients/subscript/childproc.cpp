/**
 * \file childproc.cpp
 *
 * class to handle child processes and their stdio
 *
 * \author Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 * \date Thu Aug 28 16:50:53 CEST 2008
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>	// pipe, dup, fork, exec
#include <stdlib.h>
#include <sys/types.h>	// waitpid()
#include <sys/wait.h>	// waitpid()


/// \todo Change the parsing from command_t.h to some other method
#include <subsystem/command_t.h>
#include <subsystem/error.h>

/*
int pipe(int pipefd[2]);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
pid_t fork(void);
*/


class pipe_t {
	int fd[2];
public:
	pipe_t() {
		fd[0] = fd[1] = -1;
		if (pipe(&fd) < 0) {
			perror("could not create pipe");
		}
	}

	virtual ~pipe_t() {
		if (fd[0] >=0) close(fd[0]);
		if (fd[1] >=0) close(fd[1]);
	}
	int rfd() { return fd[0]; }
	int wfd() { return fd[1]; }
};

class r_select_pipe : public pipe_t, public selectable {
public:
	r_select_pipe() : selectable(), pipe() {};
	// selectable virtuals
	virtual int getfd() { return this->rfd(); }
	virtual int getchecks() { return CHK_READ; }
};
class w_select_pipe : public pipe_t, public selectable {
public:
	r_select_pipe() : selectable(), pipe() {};
	// selectable virtuals
	virtual int getfd() { return this->wfd(); }
	virtual int getchecks() { return CHK_WRITE; }
};

#define MAX_LINELEN	256
class childproc {
//	typedef union {
//		int fd[2];
//		struct {
//			int	pout;	// read end, out of pipe
//			int	pin;	// write end into pipe
//		};
//	} pipe_fds;
	w_select_pipe *in;
	r_select_pipe *out, *err;
private:
	pid_t	pid;
	int	retstatus;	// return status filled from waitpid()
	char command[MAX_LINELEN];
	pipe_fds in;		// pipe file descriptors for stdin of child process
	pipe_fds out;		// ... stdout ...
	pipe_fds err;		// ... stderr ...
	FILE* fp[3];		// file pointers corresponding to pipes

public:
	childproc(char* cmdline) {
		pid = -1;
		retstatus=0;
		strncpy(command, cmdline, MAX_LINELEN);
	}

	~childproc() {
		if ( pid > 0 ) {
			eprintf("still running %s\n", command);
			eprintf("sending SIGTERM\n");
			kill(pid, SIGTERM);
			eprintf("waitpid(%d)\n", pid);
			waitpid(pid, &retstatus, 0);
			eprintf("child terminated\n");
		}
	}

	/**
	 * create a new process
	 *
	 * Returns the pid of the new process and gives the file-descriptors
	 * of the childs std{in,out,err} back in fd array.
	 *
	 * \return pid of new child process or negative on error.
	 */
	pid_t run() {
		char cmd[MAX_LINELEN];
		strcpy(cmd, command);

		// create pipes
		if ( pipe( in.fd) < 0 ) { eperror("could not create pipe"); return -1; }
		if ( pipe(out.fd) < 0 ) { eperror("could not create pipe"); return -1; }
		if ( pipe(err.fd) < 0 ) { eperror("could not create pipe"); return -1; }

		// fork
		if ( (pid=fork()) < 0 ) {
			eperror("could not create pipe");
			return -1;	// FIXME: do we need to close the pipes first?
		}

		if ( pid==0 ) {
			// child:

			//	connect stdout to stdout-pipe dup2(stdout.pin, STDOUT_FILENO);
			close(out.pout);		// close the read side of stdout pipe
			int ret;
			if ( (ret=dup2(out.pin, STDOUT_FILENO)) < 0 ) {
				eperror("could not dup2 stdout");
				return -1;
			}
			printf("stdout.fileno = %d\n", ret);

			//	connect stdin  to stdin-pipe  dup2(stdin.pout, STDIN_FILENO);
		 	close(in.pin);		// close the write side of stdin pipe
			if ( (ret=dup2(in.pout, STDIN_FILENO)) < 0) {	// transfer pipe output to stdin
				eperror("could not dup2 stdin");
				return -1;
			}
			printf("stdin.fileno = %d\n", ret);
		
			//	connect stderr to stderr-pipe dup2(stderr.pin, STDERR_FILENO);
			close(err.pout);		// close the read side of stderr pipe
			if ( (ret=dup2(err.pin, STDERR_FILENO)) < 0 ) {
				eperror("could not dup2 stderr");
				return -1;
			}
			printf("stderr.fileno = %d\n", ret);

			//	parse command into tokens
			char* argv[32];
			int argc=32;
			if ( command_t<VOIDFUNCPTR>::parse(cmd, strlen(cmd), argc, argv) < 0 ) {;	// FIXME: command_t is bad
				// CAUTION: stderr has been dup2ed into the pipe here
				eperror("error in parsing command line: \"%s\"", cmd);
				return -1;
			}
			argv[argc] = (char*)NULL;	// ensure NULL termination

			for (int i=0; i<5;i++) {
				printf("child exec in %d sec\n", 5-i);
				sleep(1);
			}
			printf("child exec argc=%d\n", argc);
			sleep(1);
			for (int i=0; i<argc; i++) {
				for (int j=0; j<100; j++) {
					printf("\tspeedtest = %d\n", j);
				}
				printf("\targv[%d] = \"%s\"\n", i, argv[i]);
				sleep(1);
			}
			printf("child exec argc=%d\n", argc);
			sleep(1);
			//	exec to given program
			execv(argv[0], argv);
			eperror("exec failed"); // CAUTION: stderr has been dup2ed into the pipe here
			exit(1);
		} else {
		// parent:
			close(in.pout);	// close the read side of the stdin pipe
		 	close(out.pin);	// close the write side of the stdout pipe
		 	close(err.pin);	// close the write side of the stderr pipe
		 	fp[STDIN_FILENO]  = fdopen(in.pin,"w");
			if ( fp[STDIN_FILENO] <= 0 ) {
				eperror("WARNING: open in.pin failed"); 
			}
		 	fp[STDOUT_FILENO] = fdopen(out.pout,"r");
			if ( fp[STDOUT_FILENO] <= 0 ) {
				eperror("WARNING: open out.pout failed"); 
			}
		 	fp[STDERR_FILENO] = fdopen(err.pout,"r");
			if ( fp[STDERR_FILENO] <= 0 ) {
				eperror("WARNING: open err.pout failed"); 
			}
		 	return pid;
		}
		return -1;	// never come here! (exec fail in child)
	} // end run()
	
	int isFinished(int* status = NULL) {
		int ret;
		if ( pid < 0 ) {
			errno = ECHILD;
			return -1;
		}
		ret = waitpid(pid, &retstatus, WNOHANG);
		if (status!=NULL) *status = retstatus;	// give status to caller
		if (ret > 0 ) pid=-1;	// reset pid;
	//	printf("isFinished = %d\n", ret);
		return ret; 
	}

	int status() {
		return pid;
	}

	/**
	 * File descriptors
	 */
	int in_fd() { return in.pin; }
	int out_fd() { return out.pout; }
	int err_fd() { return err.pout; }

	/**
	 * file pointers
	 */
	FILE* stdinput()  {
		if ( fp[STDOUT_FILENO] == NULL ) {
			printf("WARNING: stdin requested, giving FILE* fd = %p\n", fp[STDIN_FILENO]);
		}
		return fp[STDIN_FILENO];
	}
	FILE* stdoutput() {
		if ( fp[STDOUT_FILENO] == NULL ) {
			printf("WARNING: stdout requested, giving FILE* fd = %p\n", fp[STDOUT_FILENO]);
		}
		return fp[STDOUT_FILENO];
	}
	FILE* stderror() {
		if ( fp[STDERR_FILENO] == NULL ) {
			printf("WARNING: stdout requested, giving FILE* fd = %p\n", fp[STDERR_FILENO]);
		}
		return fp[STDERR_FILENO];
	}
};

#include <time.h>

#include <sys/select.h>

int main() {
	int ret;
	childproc grep("/bin/grep hallo");

	ret = grep.isFinished(); eperror("childproc.isFinished()=%d", ret);

	printf("running childproc...\n");
	ret = grep.run();
	printf("returned %d - running :)\n", ret);

	time_t start, now;
	char line[MAX_LINELEN];
	int linelen;
	time(&start); time(&now);
//	fprintf(grep.stdinput(), "HALLO\n");	// print to the stdin of childproc
	printf("pre loop\n");

	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(grep.out_fd(), &rfds);
	FD_SET(STDIN_FILENO, &rfds);
	int maxfd = grep.out_fd();
	fd_set rfds_save = rfds;

	struct timeval timeout;
	while ( now-start < 30 && !grep.isFinished() ) {
		timeout.tv_sec=10;
		timeout.tv_usec=0;
		rfds = rfds_save;
		printf("loop\n");
		switch ( select(maxfd+1, &rfds, NULL, NULL, &timeout) ) {
		case 0:	// timeout
			eprintf("waiting\n");
			break;
		case -1: // error
			eperror("select error");
			exit(1);
			// break;
		default:
			if ( FD_ISSET(grep.out_fd(), &rfds) ) {
				errno=0;
				if ( fgets(line, MAX_LINELEN, grep.stdoutput()) < 0 ) {	// read childs output
					eperror("read error");
					exit(1);
				}
				eperror("fgets ok");
				printf("child said: %s", line);
				ret = grep.isFinished(); eperror("childproc.isFinished()=%d", ret);
			}
			if ( FD_ISSET(STDIN_FILENO, &rfds) ) {
				if ( fgets(line, MAX_LINELEN, stdin) < 0 ) {	// read childs output
					eperror("read error");
					exit(1);
				}
				if (strlen(line) < 3) exit(0);
				printf("fgets(stdin) ok, strlen=%d\n", strlen(line));
				printf("grep.stdinput().fileno = %d\n", fileno(grep.stdinput()));
				// fprintf(grep.stdinput(), "%s", line);
				fprintf(grep.stdinput(), "asdf dies ist ein test hallo welt\n", line);
			}
		}
		time(&now);
	}
	printf("post loop\n");
	return 0;
};
