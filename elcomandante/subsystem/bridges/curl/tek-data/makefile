LIB=libsubsystem.a
LIBPATH=../../..

CC=/usr/bin/g++
CFLAG=-c
CFLAG+=`curl-config --cflags`

LDFLAGS+=-lsubsystem -L$(LIBPATH)
LDFLAGS+=`curl-config --libs`
# Program opject dependencies

ALLPROGS=tek-data

.PHONY:	all clean

all:	$(ALLPROGS) 
	@echo " DONE";

tek-data:	tek-data.o
	@echo -en " LD $@:\t";
	$(CC) -o $@ $@.o $(LDFLAGS)

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

