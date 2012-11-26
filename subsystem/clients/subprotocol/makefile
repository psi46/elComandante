#
# Subsystem Makefile
#	Dennis Terhorst
#	Sep 11 2008
#
#

LIB_BASE:=../..
# compile and link options
ifeq ($(OS),Windows_NT)
    CC=g++.exe
    CFLAGS=-c
    CCFLAGS=$(CFLAGS)
    LDFLAGS=-Lc:/Programme/MinGW/lib -lwsock32 -lws2_32
else
    CC=/usr/bin/g++
    CFLAGS=-c
    CCFLAGS=$(CFLAGS)
    LDFLAGS=
endif


ALLPROGS:=subprotocol

.PHONY:	all clean

all:	$(ALLPROGS)
	@echo " DONE";

# Program opject dependencies
DEFAULT_OBJS:=	$(LIB_BASE)/packet_t.o
subprotocol:	$(LIB_BASE)/sclient.o

.SECONDEXPANSION:
$(ALLPROGS):	$$@.o $(DEFAULT_OBJS)
#%:	%.o 
ifeq ($(OS), Windows_NT)
	@echo -n "[Windows_NT] "
	$(CC)  $@.o $(CLIENT_OBJ) $(LDFLAGS) -o $@.exe
else
	@echo -en " LD $@:\t";
	$(CC) -o $@ $^ $(LDFLAGS)
endif

%.o:	%.c | %.h
	@echo -en " cc $@:\t";
	$(CC) $(CFLAGS) $<

%.o:	%.cpp | %.h
	@echo -en " CC $@:\t";
	$(CC) $(CCFLAGS) $< -o $@

##############################################################################

clean:	$(SUBDIRS)
	@echo -en " CLEAN:\t";
ifneq ($(OS), Windows_NT)
	rm -f $(ALLPROGS) *.o $(LIB); rm -rf deb
else
	del *.o
endif
	@echo -e " CLEANED\n";

