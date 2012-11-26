LIB=libsubsystem.a
LIBPATH=../..

CC=/usr/bin/g++
CFLAGS=-c -I$(LIBPATH)

LDFLAGS+=-lsubsystem -L$(LIBPATH)

# Program opject dependencies

#ALLPROGS=main
ALLPROGS=watchdog main
PROG=main

.PHONY:	all clean

all:	$(ALLPROGS) 
	@echo " DONE";

main:	main.o ratewatch.o smoothing.o
watchdog: watchdog.o ratewatch.o smoothing.o

$(SUBDIRS):
	@cd $@;	$(MAKE);

$(ALLPROGS):
	@echo -en " LD $@:\t";
	$(CC) -o $@ $^  $(LDFLAGS)

%.o:	%.c | %.h
	@echo -en " cc $<:\t";
	$(CC) $(CFLAGS) $<

%.o:	%.cpp | %.h
	@echo -en " CC $<:\t";
	$(CC) $(CFLAGS) $<

%.o:	%.cpp
	@echo -en " CC $<:\t";
	$(CC) $(CFLAGS) -c $<

clean:
	@echo -en " CLEAN:\t";
	rm -f $(ALLPROGS) *.o

