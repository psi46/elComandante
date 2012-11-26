

# from: http://tldp.org/HOWTO/Lex-YACC-HOWTO-4.html
# NOTE: if you get an error about your compiler not being able to find 'yylval', add this to example4.l, just beneath #include <y.tab.h>: 
# extern YYSTYPE yylval;
# This is explained in the 'How Lex and YACC work internally' section.

#LEX=/usr/bin/lex
LEX=/usr/bin/flex
LEX_OPTS=

YACC=/usr/bin/yacc
YACC_OPTS= -d
# these do not work with old versions (e.g. on the cluster)
#YACC_OPTS+=--debug --verbose

PROG=grammar
TOKENS=tokens
GRAMMAR=grammar

.PHONY:	all
all:	$(PROG)

$(PROG): $(TOKENS).c $(GRAMMAR).tab.c lpt.o
	@echo -en " [CC]  "
	#cc $(TOKENS).c $(GRAMMAR).tab.c -o $(PROG)
	cc $^ -o $(PROG)



$(TOKENS).c: $(TOKENS).l
	@echo -en " [LEX] "
	$(LEX) $(LEX_OPTS) -o$(TOKENS).c $(TOKENS).l

$(GRAMMAR).tab.c: $(GRAMMAR).y
	@echo -en " [YAC] "
	yacc $(YACC_OPTS) -b $(GRAMMAR) $(GRAMMAR).y

.PHONY: smallclean
smallclean:
	@echo -en " [SMALLCLEAN] "
	rm -f $(TOKENS).c $(GRAMMAR).tab.c $(GRAMMAR).tab.h

.PHONY: clean
clean:
	@echo -en " [CLEAN] "
	rm -f $(TOKENS).c $(GRAMMAR).tab.c $(GRAMMAR).tab.h $(GRAMMAR).output *.o $(PROG)
