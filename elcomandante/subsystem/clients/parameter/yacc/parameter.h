#ifndef PARAMETER_T_H
#define PARAMETER_T_H

#define MAX_NAMELENGTH	64
#define MAX_PARAMETER	20

#define TYPE_DOUBLE	1
#define TYPE_FLOAT	2
#define TYPE_INT	3

#define BUFFER_SIZE	1024
#define TMPL_TOKEN	"./tmpl_tokens.l"
#define TMPL_GRAMMAR	"./tmpl_grammar.y"

#define AUTO_COMMAND_NAME	"auto_command"

class parameter {
	private:
		char name[MAX_PARAMETER][MAX_NAMELENGTH];
		void* pointer[MAX_PARAMETER];
		int type[MAX_PARAMETER];

		int parameter_cnt;

		char* uppercase( char *sPtr );
		char* lowercase( char *sPtr );		

		int create_tokens();
		int create_grammar();
		int do_compiling();

	public:
		parameter();
		~parameter();
		int add(char* buffer, void* var, int ptype);
		int compile();

};

#endif
