

######################################################################
PROG=action
######################################################################


######################################################################
#    Programms Used
YACC=/usr/bin/bison -v
FLEX=/usr/bin/flex --header-file=action.lex.h
GPP=/usr/bin/g++
ECHO=/bin/echo

######################################################################
#    Flags
DEBUGFLAGS=-g -DNDEBUG -Wall
CCFLAGS=$(DEBUGFLAGS) -c -I$(HOME)/lib/ -pthread
# -DUSE_PACKET_TYPE_ENUM
LDFLAGS=$(DEBUGFLAGS) -L$(HOME)/lib -lsubsystem -pthread
# no -lfl because main() is already defined


.PRECIOUS: action.tab.h lex.action_.c
.PHONY:	all clean doc
######################################################################
#    Default Target
all:	$(PROG)
	@$(ECHO) "done."

######################################################################
#    P R O J E C T   D E P E N D E N C Y   T R E E
#
$(PROG): lex.action_.o action.tab.o main.o
$(PROG): action_context.o packet_description_t.o
$(PROG): value_t.o
$(PROG): globals.o thread_t.o

value_t.h:	valueT.h valueconstT.h
action.tab.c:	value_t.h sensations.h sens_arith_t.h sens_value_t.h

######################################################################
#    R U L E S
#
$(PROG):
	@$(ECHO) -en " [LNK]\t"
	$(GPP) -o $@ $^ $(LDFLAGS) # link program

%.o: %.cpp %.h
	@$(ECHO) -en " [CC]\t"
	$(GPP) $(CCFLAGS) -o $@ $<

%.o: %.cpp
	@$(ECHO) -en " [CC]-\t"
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
	rm -f demo *.o action.tab.c action.tab.h lex.action_.c lex.action_.o action.lex.h

doc:
	@$(ECHO) -en " [DOC]\t"
	doxygen >/dev/null;

