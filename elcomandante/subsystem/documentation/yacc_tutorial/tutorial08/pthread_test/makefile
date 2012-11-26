

PROG=main
CFLAGS=-Wall
LDFLAGS=-pthread
ECHO=/bin/echo
GPP=/usr/bin/g++

.PHONY:	all clean
all:	$(PROG)
	@$(ECHO) "done."


main:	thread_test.o


$(PROG):
	@$(ECHO) -en " [LD]\t"
	$(GPP) $^ $(LDFLAGS) -o $@


%.o:	%.cpp %.h
	@$(ECHO) -en " [CC]\t"
	$(GPP) -c $(CFLAGS) $<

%.o:	%.cpp
	@$(ECHO) -en " [CC]\t"
	$(GPP) -c $(CFLAGS) $<

clean:
	@$(ECHO) -en " [CLEAN]\t"
	rm -f $(PROG) *.o
	
