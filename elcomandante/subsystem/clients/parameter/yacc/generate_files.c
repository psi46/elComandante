#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <dlfcn.h>

#define BUFFER_SIZE	1024
#define TMPL_TOKEN	"./tmpl_tokens.l"
#define TMPL_GRAMMAR	"./tmpl_grammar.y"

#define AUTO_COMMAND_NAME	"auto_command"
#define GENERATE_FILES
#define MAKE_CLEAN
#define COMPILE

typedef int (*yyparse_ptr)(void);
int *wantexit;

char* uppercase( char *sPtr ){
	char* ret = strdup(sPtr);
	int cnt=0;
	while( *ret != '\0' ){
		char c = *ret;
		*ret = toupper(c);
		ret++;
		cnt++;
	}
	return ret-cnt;
}
char* lowercase( char *sPtr ){
	char* ret = strdup(sPtr);
	int cnt=0;
	while( *ret != '\0' ){
		char c = *ret;
		*ret = tolower(c);
		ret++;
		cnt++;
	}
	return ret-cnt;
}

int main(){
	int i; // Schleifenvariable
	// TEST variable names
#define MAX_VARS 4
	
/*
subHV.cpp:
	addparameter("Refreshtime", &test1, "int");
*/

	char name[MAX_VARS][255] = {"test1", "test2", "test3", "test4"};
	char type[MAX_VARS][255] = {"int", "int", "double", "int"};
	void* pointers[MAX_VARS];

#ifdef GENERATE_FILES
	// GENERATE TOKENS
	printf(" generating tokens.l\n");
	FILE* output;
	output = fopen("./auto/tokens.l", "w");
	// open templates
	FILE* input=NULL;
	input = fopen(TMPL_TOKEN, "r");
	if(input==NULL){
		printf("\tTOKEN template %s not found\n", TMPL_TOKEN);
		return -1;
	}
	char line_buffer[BUFFER_SIZE];
	int first_mark = 0;
	while(fgets(line_buffer, sizeof line_buffer, input) != NULL ){
		if(strncmp(line_buffer, "%%", 2)==0){
			// test whether second one or not
			if(first_mark){
				// generate TOKENS	
				int i;
				for(i=0; i<MAX_VARS; i++){
					fprintf(output,"%s|%s\t\t{ return T%s; }\n", 
						lowercase(name[i]), 
						uppercase(name[i]), 
						uppercase(name[i]));
				}
				fprintf(output,"\n%s", line_buffer);
				printf("  adding tokens...\n");
			}else{	
				first_mark = 1;
				fprintf(output,"%s", line_buffer);
			}
		}else{
			fprintf(output,"%s", line_buffer);
		}
        }
	fclose(input);
	fclose(output);
	printf(" tokens.l generated\n");
	// TOKENS done
	// GENERATE GRAMMAR
	printf(" generating grammar.y\n");
	input = fopen(TMPL_GRAMMAR, "r");
	if(input==NULL){
		printf("\tGRAMMAR template %s not found\n", TMPL_TOKEN);
		return -1;
	}
	output = fopen("./auto/grammar.y", "w");
	// open templates
	int tokens_written = 0;
	int command_written = 0;
	first_mark = 0;
	while(fgets(line_buffer, sizeof line_buffer, input) != NULL ){
		if(strncmp(line_buffer, "// GLOBALS",10)==0){
			printf("  adding globals...\n");
			fprintf(output,"\n%s\n", line_buffer);
			int i;
			for(i=0; i<MAX_VARS; i++){
				fprintf(output, "%s %s=%d+42;\n",type[i], name[i], i);
			}
			fprintf(output, "\n");
			fprintf(output, "int (*callback)(int,int);\n");
		}else if(strncmp(line_buffer, "%token",6)==0 && !tokens_written){
			fprintf(output,"\n%s\n", line_buffer);
			// generate TOKENS	
			fprintf(output, "%%token ");
			int i;
			for(i=0; i<MAX_VARS; i++){
				fprintf(output,"T%s ", uppercase(name[i]));
			}
			fprintf(output, "\n\n");
			tokens_written = 1;
			printf("  adding tokens...\n");
		}else if(strncmp(line_buffer, "command:", 8)==0 && !command_written){
			fprintf(output,"\n%s", line_buffer);
			// generate command
			fprintf(output, "\t| %s\t{ YYACCEPT; }\n", AUTO_COMMAND_NAME);
			command_written = 1;	
			printf("  adding commands...\n");			
		}else if(strncmp(line_buffer, "%%", 2)==0){
			// test whether second one or not
			if(first_mark){
				fprintf(output,"%s:\n", AUTO_COMMAND_NAME);
				int i;
				for(i=0; i<MAX_VARS; i++){
					if(i==0){
						fprintf(output,"\tTSET T%s VALUE NL\n", 
							uppercase(name[i]));
					}else{
						fprintf(output,"\t| TSET T%s VALUE NL\n", 
							uppercase(name[i]));
					}
					fprintf(output,"\t  {\n");		
					fprintf(output,"\t\tprintf(\"ACTION for %s\\n\");\n", name[i]);
					fprintf(output,"\t\t%s=$3;\n",name[i]);
					fprintf(output,"\t  }\n");
				}
				fprintf(output,"\t  ;");
				// generate Command data
				fprintf(output,"\n%s", line_buffer);
				printf("  adding functions...\n");
			}else{	
				first_mark = 1;
				fprintf(output,"%s", line_buffer);
			}

		}else{
			fprintf(output,"%s", line_buffer);
		}
        }
	fclose(input);
	fclose(output);
	printf(" grammar.y generated\n");
	// GRAMMAR done
	// DO COMPILING HERE
#endif

#ifdef COMPILE
#ifdef MAKE_CLEAN
	system("cd ./auto/ && make clean");
#endif
	system("cd ./auto/ && make ");
#endif

	// OPEN LIB
	char* error;
	void *module;
	yyparse_ptr yyparse;
	int (*callback_fnct)(int,int);

	// Load dynamically loaded library
	module = dlopen("./auto/libSlowIO.so", RTLD_LAZY);
	if (!module) {
		fprintf(stderr, "Couldn't open libSlowIO.so: %s\n",
		dlerror());
		exit(1);
	}
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

// Get variables
	dlerror();
	for(i=0; i<MAX_VARS; i++){
		pointers[i] = dlsym(module, name[i]);		
		if ((error = dlerror())) {
			fprintf(stderr, "Couldn't find %s: %s\n", name[i], error);
			exit(1);
		}
	}

// Run application
	*wantexit=0;
	while (! (*wantexit)) {
		printf("%d, %d, %lf, %d> ", *(int*)pointers[0], *(int*)pointers[1], *(double*)pointers[2], *(int*)pointers[3]); fflush(stdout);
		yyparse(); // PARSE INPUT
	}
	printf("bye.\n");
	// All done, close things cleanly
	printf("close module\n");
	dlclose(module);
	return 0;
}
