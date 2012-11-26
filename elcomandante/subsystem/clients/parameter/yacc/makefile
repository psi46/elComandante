CC=gcc
CPP=g++

ALLPROGS=generate_files
OBJ=parameter.o

LDFLAGS=-ldl -lc

all:	$(ALLPROGS)
	@echo " DONE";
	
generate_files:	generate_files.o $(OBJ)
	@echo -en " LD $@:\t";
	$(CPP) -o $@ $@.o $(OBJ) $(LDFLAGS)

%.o:	%.cpp | %.h
	@echo -en " CC $<:\t";
	$(CPP) -Wall -I$(LIBPATH) -c $<

clean:
	@echo -en " CLEAN:\t";
	rm -f $(ALLPROGS) *.o

