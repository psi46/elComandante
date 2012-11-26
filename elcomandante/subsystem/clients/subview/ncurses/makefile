
PROG=test
CFLAGS=-Wall
LDFLAGS=-lncurses
GPP=/usr/bin/g++

$(PROG): ncurses_screen.o ncurses_element.o

.PHONY: all clean

all:	$(PROG)
	@echo "done.";

$(PROG):	$(PROG).o
	@echo -en " [LD]\t$@: "
	$(GPP) $(LDFLAGS) $^ -o $@

%.o:	%.cpp %.h
	@echo -en " [CC]\t$@: "
	$(GPP) $(CFLAGS) -c $<

%.o:	%.cpp
	@echo -en " [CC]\t$@: "
	$(GPP) $(CFLAGS) -c $<

clean:
	@echo -en " [CLEAN]\t"
	rm -f *.o $(PROG)
