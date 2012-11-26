

PROG=example5

.PHONY: all smallclean
all:	$(PROG)
$(PROG):	$(PROG)_gen.lex.yy.c $(PROG)_gen.tab.c
	@echo -en " Compile:\t"
	gcc $? -o $(PROG)


# BUILD GRAMMAR
%_gen.tab.c:	%.y
	@echo -en " Grammar:\t"
	yacc -d -b $(PROG)_gen -v $(PROG).y

# BUILD LEXER
%_gen.lex.yy.c:	%.l
	@echo -en " Lexer:  \t"
	flex -o $@ $^

smallclean:
	@echo -en " Clean:  \t"
	rm -f $(PROG)_gen*

.PHONY: clean
clean:	smallclean
	rm -f $(PROG) *.o y.tab.* lex.y*
