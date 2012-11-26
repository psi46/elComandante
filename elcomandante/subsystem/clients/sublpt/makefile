LIB=libsubsystem.a
LIBPATH=../..
INCPATH=$(LIBPATH)/subsystem

# Program opject dependencies

#PROG=sublpt
PROG=flex_sublpt
OBJ=lpt.o daemon.o


#
# Compile Environment
#
ifeq ($(OS),Windows_NT)
CC=gcc.exe
GPP=g++.exe
CFLAGS=-I$(LIBPATH)
CCFLAGS=$(CFLAGS)
LDFLAGS=-Lc:/Programme/MinGW/lib -lwsock32 -lws2_32 -L$(LIBPATH) -lsubsystem
else
CC=/usr/bin/gcc
GPP=/usr/bin/g++
CFLAGS=-I$(LIBPATH)
CCFLAGS=$(CFLAGS)
#LDFLAGS=-L$(LIBPATH) -lsubsystem
endif


.PHONY: all smallclean
all:	$(PROG)
$(PROG):	$(PROG)_gen.lex.yy.c $(PROG)_gen.tab.c $(OBJ)
	@echo -en " Compile:\t"
	gcc $(CFLAGS) $^ $(LDFLAGS) -o $(PROG)

%.o:	%.c
	@echo -en " cc $<:\t";
	$(CC) $(CFLAGS) -c $< -o $@

# BUILD GRAMMAR
%_gen.tab.c:	%.y
	@echo -en " Grammar:\t"
	yacc -d -b $(PROG)_gen -v $(PROG).y

# BUILD LEXER
%_gen.lex.yy.c:	%.l
	@echo -en " Lexer:  \t"
	flex --header-file=$(PROG)_gen.lex.yy.h -o $@ $^

smallclean:
	@echo -en " Clean:  \t"
	rm -f $(PROG)_gen*

.PHONY: clean
clean:	smallclean
	rm -f $(PROG) *.o y.tab.* lex.y*
