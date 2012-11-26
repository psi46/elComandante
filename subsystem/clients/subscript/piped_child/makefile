
PROG=pipe
CC=g++
CFLAGS=-Wall -g -I ../../..
LDFLAGS=-g -L ../../.. -lsubsystem


######################################

.PHONY: all clean

ALLPROGS=$(PROG) test-reader test-writer test-writer-err
# default target
all:	$(ALLPROGS)

# additional prerequisites
pipe:	util.o child_t.o child_info_t.o selectable.o selectable_sclient.o

# specialized rules
test-reader: test-reader.c
	$(CC) $^ -o $@

test-writer: test-writer.c
	$(CC) $^ -o $@

test-writer-err: test-writer-err.c
	$(CC) $^ -o $@

######################################
# general rules
######################################
$(PROG):	$(PROG).o
	@echo -en " link    $@\t"
	$(CC) $^ -o $@ $(LDFLAGS)

%.o:	%.c %.h
	@echo -en " compile $@\t"
	$(CC) -c $(CFLAGS) $<

%.o:	%.cpp %.h
	@echo -en " Compile $@\t"
	$(CC) -c $(CFLAGS) $<

######################################
clean:
	@echo -en " clean  \t"
	rm -f $(ALLPROGS) *.o *.gch
	
