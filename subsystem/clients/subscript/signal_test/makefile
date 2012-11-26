PROG=sigtest
CC=g++
CFLAGS=-Wall -g
LDFLAGS=-g

$(PROG):	$(PROG).o
	@echo -en " link    $@\t"
	$(CC) $^ -o $@ $(LDFLAGS)

%.o:	%.c
	@echo -en " compile $@\t"
	$(CC) -c $(CFLAGS) $^
