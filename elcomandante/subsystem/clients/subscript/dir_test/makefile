LIBPATH=../../..

PROG=dirtest

GPP=/usr/bin/g++
CCFLAGS=-I$(LIBPATH)
LDFLAGS=-L$(LIBPATH) -lsubsystem

.PHONY:	all clean doc

all:	$(PROG)
	@echo " DONE";

# additional prerequisites
dirtest: util.o


######### RULES ############

$(PROG):	$(PROG).o
	@echo -en " LD\t";
	$(GPP) $^ $(LDFLAGS) -o $@

%.o:	%.cpp
	@echo -en " CC\t";
	$(GPP) -c $< $(CCFLAGS)

doc:
	@echo -e " FIXME: DOC TARGET N/A";
	false;

clean:
	@echo -en " CLEAN:\t";
	rm -f $(PROG) *.o
