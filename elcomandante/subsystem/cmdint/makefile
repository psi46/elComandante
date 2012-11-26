
PROG=test2
OBJ=convert.o

CC=g++

CCFLAGS=-Wall -O0
# -I$(LIBPATH)
LDFLAGS=-Wall
.PHONY:	all clean install bin deb

all:	$(PROG)

$(PROG):	$(PROG).o $(OBJ)
	@echo -en " LD $@:\t"
	$(CC) $(LDFLAGS) $^ -o $@
	@echo -e " DONE"


%.o:    %.c | %.h
	@echo -en " cc $<:\t";
	$(CC) $(CCFLAGS) -c $<

%.o:    %.cpp | %.h
	@echo -en " CC $<:\t";
	$(CC) $(CCFLAGS) -c $<

%.o:    %.cpp
	@echo -en " CC $<:\t";
	$(CC) $(CCFLAGS) -c $<

install:
bin:
deb:

clean:
	@echo -en " CLEAN:\t";
ifneq ($(OS), Windows_NT)
	rm -f $(PROG) *.o $(LIB)
else
	del $(PROG) *.o
endif
	@echo -e " CLEANED";

