
YACC=/usr/bin/bison -v
FLEX=/usr/bin/flex
GPP=/usr/bin/g++
ECHO=/bin/echo
CCFLAGS=-g -c -I$(HOME)/lib/ -pthread
# -DUSE_PACKET_TYPE_ENUM
LDFLAGS=-g -L$(HOME)/lib -lsubsystem -pthread

PROG=test

# no -lfl because main() is already defined
.PHONY:	all clean doc
all:	$(PROG)
	@$(ECHO) "done."

$(PROG): test.o

$(PROG):
	@$(ECHO) -en " [LNK]\t"
	$(GPP) -o $@ $^ $(LDFLAGS) # link program

%.o: %.cpp
	@$(ECHO) -en " [CC]\t"
	$(GPP) $(CCFLAGS) -o $@ $<

lex.%_.o: lex.%_.c %.tab.h
	@$(ECHO) -en " [cc]\t"
	$(GPP) $(CCFLAGS) -o $@ $<	# compile lexer

lex.%_.c: %.l
	@$(ECHO) -en " [FLEX]\t"
	$(FLEX) $<	# create lexer

%.tab.o: %.tab.c
	@$(ECHO) -en " [cc]\t"	
	$(GPP) $(CCFLAGS) -o $@ $<	# compile parser

%.tab.c %.tab.h: %.y
	@$(ECHO) -en " [YACC]\t"
	$(YACC) $<	# create parser


clean:
	@$(ECHO) -en " [CLEAN]\t"
	rm -f demo *.o action.tab.c action.tab.h lex.action_.c lex.action_.o

doc:
	@$(ECHO) -en " [DOC]\t"
	doxygen >/dev/null;

