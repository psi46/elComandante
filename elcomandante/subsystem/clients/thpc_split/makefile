LIB=libsubsystem.a
LIBPATH=../..

CC=/usr/bin/g++
CFLAG=-c

LDFLAGS+=-lsubsystem -L$(LIBPATH)

# Program opject dependencies

ALLPROGS=thpc_split

.PHONY:	all clean

all:	$(ALLPROGS) 
	@echo " DONE";

$(SUBDIRS):
	@cd $@;	$(MAKE);

thpc_split:	thpc_split.o 
	@echo -en " LD $@:\t";
	$(CC) -o $@ $@.o  $(LDFLAGS)

%.o:	%.c | %.h
	@echo -en " cc $<:\t";
	$(CC) -I$(LIBPATH) -c $<

%.o:	%.cpp | %.h
	@echo -en " CC $<:\t";
	$(CC) -I$(LIBPATH) -c $<

%.o:	%.cpp
	@echo -en " CC $<:\t";
	$(CC) -I$(LIBPATH) -c $<

clean:
	@echo -en " CLEAN:\t";
	rm -f $(ALLPROGS) *.o

