%{
#include <stdio.h>
#include <string.h>
#include "lpt.h"
#include <time.h>

// GLOBALS
int yydebug=0;
int wantexit=0;
int parfd=-1;
char bitpat=0;

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
void fstat_print(int fd) {
	struct stat sb;

	if (fstat(fd, &sb) == -1) {
		perror("stat");
		exit(EXIT_SUCCESS);
	}

	printf("File type:                ");

	switch (sb.st_mode & S_IFMT) {
	case S_IFBLK:  printf("block device\n");            break;
	case S_IFCHR:  printf("character device\n");        break;
	case S_IFDIR:  printf("directory\n");               break;
	case S_IFIFO:  printf("FIFO/pipe\n");               break;
	case S_IFLNK:  printf("symlink\n");                 break;
	case S_IFREG:  printf("regular file\n");            break;
	case S_IFSOCK: printf("socket\n");                  break;
	default:       printf("unknown?\n");                break;
	}

	printf("I-node number:            %ld\n", (long) sb.st_ino);

	printf("Mode:                     %lo (octal)\n", (unsigned long) sb.st_mode);

	printf("Link count:               %ld\n", (long) sb.st_nlink);
	printf("Ownership:                UID=%ld   GID=%ld\n", (long) sb.st_uid, (long) sb.st_gid);

	printf("Preferred I/O block size: %ld bytes\n", (long) sb.st_blksize);
	printf("File size:                %lld bytes\n", (long long) sb.st_size);
	printf("Blocks allocated:         %lld\n", (long long) sb.st_blocks);

	printf("Last inode change:        %s", ctime(&sb.st_ctime));
	printf("Last file access:         %s", ctime(&sb.st_atime));
	printf("Last file modification:   %s", ctime(&sb.st_mtime));
	fflush(stdout);
}


void yyerror(const char *str) {
	fprintf(stderr, "parser error: %s\n",str);
}

int yywrap() {
	return 1;
} 

#include <signal.h>
#define INTERVAL_SEC 3

void sigalrm_handler(int sig) {
	char line[256];
	snprintf(line, 256, "%ld %d.0 %d.0 %d.0 %d.0 %d.0 %d.0 %d.0 %d.0\n",
		time(NULL),
		(bitpat>>0)&1, (bitpat>>1)&1, (bitpat>>2)&1, (bitpat>>3)&1, 
		(bitpat>>4)&1, (bitpat>>5)&1, (bitpat>>6)&1, (bitpat>>7)&1);
	printf("%s", line); fflush(stdout);
	alarm(INTERVAL_SEC);
	return;
}

int main(void) {
	wantexit=0;

	//fstat_print(STDIN_FILENO);
	struct sigaction time_handler;
	time_handler.sa_handler = sigalrm_handler;
	sigemptyset(&time_handler.sa_mask);
	time_handler.sa_flags= SA_RESTART;	// do not disturb lex input
	if (sigaction(SIGALRM, &time_handler, NULL)) {
		perror("could not set SIGALRM handler");
		exit(EXIT_FAILURE);
	}

	parfd = lpt_open("/dev/parport0"); // OPEN
	sigalrm_handler(SIGALRM);
	while (!wantexit) {
		//printf("%ld waiting for commands...\n", time(NULL)); fflush(stdout);
		yyparse(); // PARSE INPUT
		//printf("%ld yyparse returned, wantexit=%d\n", time(NULL), wantexit ); fflush(stdout);
	}
	fprintf(stderr, "closing parport\n");
	lpt_close(parfd); // CLOSE
	return 0;
} 

%}

%start command
%union {
	int integer;
	double real;
	char* words;
}
%token <integer> INTEGER
%token <real>	 REAL
%type  <real>	 VALUE
%token <integer> TDEBUG
%token <words>	 TUNKNOWN
%token TSET TOUTPUT TGAIN TPOLARITY TOFFSET TDIGITAL NL TEXIT TALL

%%

/*commands: *//* empty */
/*	| commands command
	;*/

command:  set_output	{ YYACCEPT; }
	| set_gain	{ YYACCEPT; }
	| set_polarity	{ YYACCEPT; }
	| set_offset	{ YYACCEPT; }
	| set_digital	{ YYACCEPT; }
	| NL		{ YYACCEPT; }
	| debug_switch	{ YYACCEPT; }
	| TEXIT NL	{ YYACCEPT; }
	| TUNKNOWN	{ YYABORT; }

debug_switch: TDEBUG NL
	{
		yydebug=$1;
		fprintf(stderr, "debuging turned %s\n", ($1?"on":"off"));
	}

/*
 * I don't know jet, if this would be a better solution:
 *
set_command:
	TSET WHAT NUMBER NUMBER
	{
	switch ($2) {
	case TOUTPUT:   printf("\tChannel %d set to %d\n", $3, $4); break;
	case TGAIN:     printf("\tGain of channel %d set to %d\n", $3, $4); break;
	case TPOLARITY: printf("\tPolarity of channel %d set to %d\n", $3, $4); break;
	case TOFFSET:   printf("\tOffset of channel %d set to %d\n", $3, $4); break;
	case TDIGITAL:  printf("\tChannels 0x%X set to 0x%X\n", $3, $4); break;
	default:
		return 1;
	}
	;
*/

VALUE:	  REAL { $$=$1; }
	| INTEGER { $$=(double)$1; }

set_output:
	TSET TOUTPUT INTEGER VALUE NL
	{
		char mask = (1<<$3);
		if ($4 > .5) {
			bitpat |= (1<<$3); // set bit
			printf("%ld Ch%d output %f\n", time(NULL), $3, 1.0); fflush(stdout);
		} else {
			bitpat &= ~(1<<$3); // clear bit
			printf("%ld Ch%d output %f\n", time(NULL), $3, 0.0); fflush(stdout);
		}
		lpt_setdata(parfd, bitpat);
	}
	;

set_gain:
	TSET TGAIN INTEGER VALUE NL
	{
		printf("%ld Ch%d gain %lf\n", time(NULL), $3, $4); fflush(stdout);
	}
	;

set_polarity:
	TSET TPOLARITY INTEGER VALUE NL
	{
		printf("%ld Ch%d polarity %lf\n", time(NULL), $3, $4); fflush(stdout);
	}
	;

set_offset:
	TSET TOFFSET INTEGER VALUE NL
	{
		printf("%ld Ch%d offset %lf\n", time(NULL), $3, $4); fflush(stdout);
	}
	;

set_digital:
	TSET TDIGITAL INTEGER INTEGER NL
	{
		bitpat &= ~($3);	// clear masked bits
		bitpat |= ($3 & $4);    // set masked bits
		lpt_setdata(parfd, bitpat);
		printf("%ld Ch0x%X digital 0x%X\n", time(NULL), $3, bitpat); fflush(stdout);
	}
	;
%%


