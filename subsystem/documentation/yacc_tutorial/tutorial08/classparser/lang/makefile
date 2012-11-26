# from: http://tldp.org/HOWTO/Lex-YACC-HOWTO-4.html
# NOTE: if you get an error about your compiler not being able to find 'yylval', add this to example4.l, just beneath #include <y.tab.h>: 
# extern YYSTYPE yylval;
# This is explained in the 'How Lex and YACC work internally' section.

ECHO=/bin/echo
#LEX=/usr/bin/lex
LEX=/usr/bin/flex
YACC=/usr/bin/yacc

CC=/usr/bin/gcc

PROG=grammar
TOKENS=tokens
GRAMMAR=grammar

.PHONY:	all
all:	$(PROG) smallclean

$(PROG): $(GRAMMAR).tab.o $(TOKENS).o
	@$(ECHO) -en " [LD]  "
	$(CC) $^ -o $(PROG)

%.o:	%.c
	@$(ECHO) -en " [CC]  "
	$(CC) -c $<


$(TOKENS).c: $(TOKENS).l
	@$(ECHO) -en " [LEX] "
	$(LEX) -o $(TOKENS).c $(TOKENS).l

$(GRAMMAR).tab.c: $(GRAMMAR).y
	@$(ECHO) -en " [YAC] "
	$(YACC) -b $(GRAMMAR) -d $(GRAMMAR).y

.PHONY: smallclean
smallclean:
	@$(ECHO) -en " [SMALLCLEAN] "
	rm -f $(TOKENS).c $(GRAMMAR).tab.c $(GRAMMAR).tab.h

.PHONY: clean
clean:
	@$(ECHO) -en " [CLEAN] "
	rm -f $(TOKENS).c $(GRAMMAR).tab.c $(GRAMMAR).tab.h $(GRAMMAR).output *.o $(PROG)
