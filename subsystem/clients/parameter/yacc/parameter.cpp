#include "parameter.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <dlfcn.h>
#include <ctype.h>

char* parameter::uppercase( char *sPtr ){
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
char* parameter::lowercase( char *sPtr ){
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

parameter::parameter(){
	parameter_cnt = 0;
}
parameter::~parameter(){

}
int parameter::add(char* buffer, void* var, int ptype){
	sprintf(name[parameter_cnt], "%s", buffer);
	pointer[parameter_cnt] = var;
	type[parameter_cnt] = ptype;
	return 0;
}

int parameter::create_tokens(){
	int i; // Schleifenvariable
	// GENERATE TOKENS
	printf(" generating tokens.l\n");
	FILE* output=NULL;
	FILE* input=NULL;
	output = fopen("./auto/tokens.l", "w");
	input = fopen(TMPL_TOKEN, "r");  // open templates
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
				for(i=0; i<MAX_PARAMETER; i++){
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
	return 0;
}
int parameter::create_grammar(){
	// GENERATE GRAMMAR
	printf(" generating grammar.y\n");
	FILE* output=NULL;
	FILE* input=NULL;
	input = fopen(TMPL_GRAMMAR, "r");
	if(input==NULL){
		printf("\tGRAMMAR template %s not found\n", TMPL_TOKEN);
		return -1;
	}
	output = fopen("./auto/grammar.y", "w");
	// open templates
	int tokens_written = 0;
	int command_written = 0;
	int first_mark = 0;
	char line_buffer[BUFFER_SIZE];
	while(fgets(line_buffer, sizeof line_buffer, input) != NULL ){
		if(strncmp(line_buffer, "// GLOBALS",10)==0){
			printf("  adding globals...\n");
			fprintf(output,"\n%s\n", line_buffer);
			int i;
			for(i=0; i<MAX_PARAMETER; i++){
				fprintf(output, "%s %s=%d+42;\n",type[i], name[i], i);
			}
			fprintf(output, "\n");
			fprintf(output, "int (*callback)(int,int);\n");
		}else if(strncmp(line_buffer, "%token",6)==0 && !tokens_written){
			fprintf(output,"\n%s\n", line_buffer);
			// generate TOKENS	
			fprintf(output, "%%token ");
			int i;
			for(i=0; i<MAX_PARAMETER; i++){
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
				for(i=0; i<MAX_PARAMETER; i++){
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
	return 0;
}
int parameter::do_compiling(){
	return system("cd ./auto/ && make clean all");
}
int parameter::compile(){
	int ret = 0;
	ret+=create_tokens();
	ret+=create_grammar();
	ret+=do_compiling();
	return ret;
}
