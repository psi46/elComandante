/* demo_dynamic.c -- demonstrate dynamic loading and
 *    use of the "hello" routine */


/* Need dlfcn.h for the routines to
 *    dynamically load libraries */
#include <dlfcn.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* Note that we don't have to include "libhello.h".
 *    However, we do need to specify something related;
 *       we need to specify a type that will hold the value
 *          we're going to get from dlsym(). */

/* The type "simple_demo_function" describes a function that
 *    takes no arguments, and returns no value: */

//typedef void (*simple_demo_function)(void);
//typedef int (*lpt_open_ptr)(const char*);
//typedef int (*lpt_close_ptr)(void);//int);
typedef int (*yyparse_ptr)(void);

int *wantexit;

int main(void) {
	const char *error;
	void *module;
//	simple_demo_function demo_function;
	yyparse_ptr yyparse;
//	lpt_open_ptr lpt_open;
//	lpt_close_ptr lpt_close;

	/* Load dynamically loaded library */
	module = dlopen("libSlowIO.so", RTLD_LAZY);
	if (!module) {
		fprintf(stderr, "Couldn't open libhello.so: %s\n",
		dlerror());
		exit(1);
	}


/*
	printf("Get symbol lpt_open\n");
	dlerror();
	lpt_open = dlsym(module, "lpt_open");
	if ((error = dlerror())) {
		fprintf(stderr, "Couldn't find lpt_open: %s\n", error);
		exit(1);
	}


	printf("Get symbol lpt_close\n");
	dlerror();
	lpt_close = dlsym(module, "lpt_close");
	if ((error = dlerror())) {
		fprintf(stderr, "Couldn't find lpt_close: %s\n", error);
		exit(1);
	}
*/

	printf("Get symbol yyparse\n");
	dlerror();
	yyparse = dlsym(module, "yyparse");
	if ((error = dlerror())) {
		fprintf(stderr, "Couldn't find yyparse: %s\n", error);
		exit(1);
	}

	printf("Get symbol wantexit\n");
	dlerror();
	wantexit = dlsym(module, "wantexit");
	if ((error = dlerror())) {
		fprintf(stderr, "Couldn't find wantexit: %s\n", error);
		exit(1);
	}


///
	*wantexit=0;

//	lpt_open("/dev/parport0"); // OPEN
	while (! (*wantexit)) {
		printf("> "); fflush(stdout);
		yyparse(); // PARSE INPUT
	}
	printf("bye.\n");
//	lpt_close(); // CLOSE
	//return 0;

///
	/* All done, close things cleanly */
	printf("close module\n");
	dlclose(module);
	return 0;
}
