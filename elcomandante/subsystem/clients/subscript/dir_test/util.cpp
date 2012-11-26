#include "util.h"

#include <signal.h>
#include <errno.h>
#include <string.h>
int register_handler(int sig, void (*func)(int)) {
	struct sigaction sa;
	//printf("registering handler for signal %d\n", sig);
	sa.sa_handler = func;
	// see man sigsetops(3)
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;

	return sigaction(sig, &sa, NULL);
}


//////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
void stat_print(char* filename) {
        struct stat sb;

        if (stat(filename, &sb) == -1) {
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

