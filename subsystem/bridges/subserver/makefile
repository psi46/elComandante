LIBPATH=../..


ifeq ($(OS),Windows_NT)
CC=g++.exe
CFLAG=-c
LDFLAGS+=-Lc:/Programme/MinGW/lib -lwsock32 -lws2_32
else
CC=/usr/bin/g++
CFLAG=-c
LDFLAGS=../../packet_t.o ../../sclient.o
endif

ALLPROGS=single_bridge_ow single_bridge_tw

.PHONY:	all clean

all:	$(ALLPROGS) 
	@echo " DONE";

single_bridge_ow:	single_bridge.o $(SINGLE_BRIDGE_OBJ)
	@echo -en " LD $@:\t";
	$(CC) -o $@ single_bridge.o $(SINGLE_BRIDGE_OBJ) $(LDFLAGS)

single_bridge_tw:	single_bridge_tw.o $(SINGLE_BRIDGE_OBJ)
	@echo -en " LD $@:\t";
	$(CC) -o $@ $@.o $(SINGLE_BRIDGE_OBJ) $(LDFLAGS)

single_bridge_tw.o:	single_bridge.cpp
	@echo -en " cc $<:\t";
	$(CC) -I$(LIBPATH) -DDUAL_WAY_BRIDGE -c $< -o single_bridge_tw.o

%.o:	%.c | %.h
	@echo -en " cc $<:\t";
	$(CC) -I$(LIBPATH) -c $<

%.o:	%.cpp | %.h
	@echo -en " CC $<:\t";
	$(CC) -I$(LIBPATH) -c $<

clean:
	@echo -en " CLEAN:\t";
ifneq ($(OS), Windows_NT)
	rm -f $(ALLPROGS) *.o $(LIB)
else
	del *.o
endif

