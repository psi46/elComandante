LIB=libsubsystem.a
LIBPATH=../..

ifeq ($(OS),Windows_NT)
CC=g++.exe
LDFLAGS=-Lc:/Programme/MinGW/lib -lwsock32 -lws2_32
else
CC=/usr/bin/g++
endif
CFLAG=-c

LDFLAGS+=-lsubsystem -L$(LIBPATH)

# Program opject dependencies
CLIENT_IRC_OBJ=irc_client.o

ALLPROGS=client_irc

.PHONY:	all clean

all:	$(ALLPROGS) 
	@echo " DONE";

$(SUBDIRS):
	@cd $@;	$(MAKE);

client_irc:	client_irc.o $(CLIENT_IRC_OBJ)
	@echo -en " LD $@:\t";
	$(CC) -o $@ $@.o $(CLIENT_IRC_OBJ) $(LDFLAGS)

%.o:	%.c | %.h
	@echo -en " cc $<:\t";
	$(CC) -I$(LIBPATH) -c $<

%.o:	%.cpp | %.h
	@echo -en " CC $<:\t";
	$(CC) -I$(LIBPATH) -c $<

clean:
	@echo -en " CLEAN:\t";
ifneq ($(OS), Windows_NT)
	rm -f $(ALLPROGS) *.o
else
	del *.o
endif

