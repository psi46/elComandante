LIB=libsubsystem.a
LIBPATH=../..

CC=/usr/bin/g++
CFLAGS=-c -I$(LIBPATH)

LDFLAGS+=-lsubsystem -L$(LIBPATH)

# Program opject dependencies

#ALLPROGS=main
ALLPROGS=sysmon

.PHONY:	all clean

all:	$(ALLPROGS) 
	@echo " DONE";

sysmon:	sysmon.o

$(SUBDIRS):
	@cd $@;	$(MAKE);

$(ALLPROGS):
	@echo -en " LD $@:\t";
	$(CC) -o $@ $^  $(LDFLAGS)

%.o:	%.c
	@echo -en " cc $<:\t";
	$(CC) $(CFLAGS) $<

%.o:	%.cpp
	@echo -en " CC $<:\t";
	$(CC) $(CFLAGS) $<

%.o:	%.cpp
	@echo -en " CC $<:\t";
	$(CC) $(CFLAGS) -c $<

clean:
	@echo -en " CLEAN:\t";
	rm -f $(ALLPROGS) *.o

