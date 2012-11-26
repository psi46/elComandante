LIBPATH=../..

PROG=aggregator
OBJ=


GPP=/usr/bin/g++
CCFLAGS=-I$(LIBPATH)
LDFLAGS=-L$(LIBPATH) -lsubsystem

.PHONY:	all clean doc

all:	$(PROG)
	@echo " DONE";

$(PROG):	$(PROG).o $(OBJ)
	@echo -en " LD\t";
	$(GPP) $^ $(LDFLAGS) -o $@

%.o:	%.cpp | %.h
	@echo -en " CC\t";
	$(GPP) -c $< $(CCFLAGS)

doc:
	@echo -e " FIXME: DOC TARGET N/A";
	false;

clean:
	@echo -en " CLEAN:\t";
	rm -f $(PROG) $(OBJ) $(PROG).o
