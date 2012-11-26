LIBPATH=../..

CC=/usr/bin/gcc
CPP=/usr/bin/g++

CFLAG=-c
#LDFLAGS+=-lsubsystem -L$(LIBPATH)

# Program opject dependencies
OBJ=parameter_t.o

ALLPROGS=parameter_test

.PHONY:	all clean

all:	$(ALLPROGS)
	@echo " DONE";
	
parameter_test:	parameter_test.o $(OBJ)
	@echo -en " LD $@:\t";
	$(CPP) -o $@ $@.o $(OBJ) $(LDFLAGS)

%.o:	%.cpp | %.h
	@echo -en " CC $<:\t";
	$(CPP) -I$(LIBPATH) -c $<

clean:
	@echo -en " CLEAN:\t";
	rm -f $(ALLPROGS) *.o

