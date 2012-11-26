
################################################################################
# Compile/Link Options
LDFLAGS=
CCFLAGS=-Wall

PROG=main
GPP=/usr/bin/g++

################################################################################
# YACC/LEX Stuff

# from: http://tldp.org/HOWTO/Lex-YACC-HOWTO-4.html
# NOTE: if you get an error about your compiler not being able to find 'yylval', add this to example4.l, just beneath #include <y.tab.h>: 
# extern YYSTYPE yylval;
# This is explained in the 'How Lex and YACC work internally' section.

LEX=/usr/bin/flex
LEX_OPTS=

YACC=/usr/bin/bison
YACC_OPTS=
# these do not work with old versions (e.g. on the cluster)
#YACC_OPTS+=--debug --verbose

ECHO=$(shell which echo)

TOKENS=subscript
GRAMMAR=subscript

################################################################################
.PHONY: all smallclean clean
all:	$(PROG)

################################################################################
# main prog dependencies
$(PROG):	action_context_t.h sensations.h
$(PROG):	valuei_t.o valuesconst_t.o valued_t.o packet_description_t.o
$(PROG):	subscript.o subscript.tab.o

################################################################################
$(PROG):	$(PROG).o
	@$(ECHO) -en " [LD]  "
	$(GPP) $(LDFLAGS) $^ -o $@

$(PROG).o:	$(PROG).cpp
	@$(ECHO) -en " [CC]+ "
	$(GPP) $(CCFLAGS) -c $<

%.o:	%.cpp %.h
	@$(ECHO) -en " [CC]  "
	$(GPP) $(CCFLAGS) -c $<

################################################################################
# YACC AND LEX PARSING
%.o:	%.c
	@$(ECHO) -en " [cc]  "
	$(GPP) $(CFLAGS) -c $^

$(TOKENS).c: $(TOKENS).l | $(GRAMMAR).tab.c
	@$(ECHO) -en " [LEX] "
	$(LEX) $(LEX_OPTS) -o$(TOKENS).c $(TOKENS).l

$(GRAMMAR).tab.c: $(GRAMMAR).y
	@$(ECHO) -en " [YAC] "
	$(YACC) $(YACC_OPTS) -b $(GRAMMAR) $(GRAMMAR).y

################################################################################
# CLEANUP
smallclean:
	@$(ECHO) -en " [SMALLCLEAN] "
	rm -f $(TOKENS).c $(GRAMMAR).tab.c $(GRAMMAR).tab.h

clean:	smallclean
	@$(ECHO) -en " [CLEAN] "
	rm -f *.o $(PROG)
