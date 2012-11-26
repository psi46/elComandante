#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>	// basename(), dirname()
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "util.h"
#define MAX_NAMELEN	512

int add_script(char* filename) {
	char name[MAX_NAMELEN]={0};
	char* ptr;
        if ( gethostname(name, MAX_NAMELEN) < 0 ) name[MAX_NAMELEN-1]=0; // gethostname and ensure term \0
        if ((ptr=strchr(name,'.'))!= NULL) *ptr=0; // cut domain part
	snprintf(&(name[strlen(name)]), MAX_NAMELEN-strlen(name)-1, ":%s", filename);	// append :filename
	fprintf(stderr, "adding -%s- to scripts.\n", name);
	return 0;
}

int wanttoadd(char* filename) {
	struct stat sb;
        if (stat(filename, &sb) == -1) {
		fprintf(stderr, "stat \"%s\" failed: ", filename);
                perror("");
                return 0;
        }

	if ( S_ISREG(sb.st_mode) ) {
	 	if ( sb.st_mode & S_IXOTH )
			 { return 1; }
		if ( (sb.st_mode & S_IXGRP) && (sb.st_gid==getgid()) )
			 { return 1; }
		if ( (sb.st_mode & S_IXUSR) && (sb.st_uid==getuid()) )
			 { return 1; }
	}
	return 0;
}

int load_scripts(char* directory) {
	DIR *dirp;
	struct dirent *dp;
	
	if ((dirp = opendir (directory)) == NULL) {
		fprintf(stderr, "Cannot open directory %s\n", directory);
		return EXIT_FAILURE;
	}

	do {
		errno = 0;
		if ((dp = readdir(dirp)) != NULL) {
			//if (strcmp(dp->d_name, arg) != 0)
			//	continue;
			//printf("\n\n ---=== %s ===---\n", dp->d_name);
			//stat_print(dp->d_name);
			if ( wanttoadd(dp->d_name) ) add_script(dp->d_name);

		}
	} while (dp != NULL);

	if (errno != 0)
		perror("error reading directory");
	closedir(dirp);

	return EXIT_SUCCESS;
}

int main(void) {
	load_scripts(".");
	printf("fertig.\n");
	return 0;
}
