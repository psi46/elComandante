LIBPATH=../..

CC=/usr/bin/gcc
CPP=/usr/bin/g++

CFLAG=-c
LDFLAGS+=-lsubsystem -L$(LIBPATH)

# Program opject dependencies
CONFIGURE_OBJ=config.o conf.o tree.o strbuf.o
SUB_CONFIG_OBJ=sub_config.o

ALLPROGS=configure config_test

.PHONY:	all clean

all:	$(ALLPROGS) 
	@echo " DONE";

configure:	configure.o $(CONFIGURE_OBJ)
	@echo -en " LD $@:\t";
	$(CPP) -o $@ $@.o $(CONFIGURE_OBJ) $(LDFLAGS)

config_test:	config_test.o $(SUB_CONFIG_OBJ)
	@echo -en " LD $@:\t";
	$(CPP) -o $@ $@.o $(SUB_CONFIG_OBJ) $(LDFLAGS)
	
%.o:	%.c | %.h
	@echo -en " cc $<:\t";
	$(CC) -I$(LIBPATH) -c $<

%.o:	%.cpp | %.h
	@echo -en " CC $<:\t";
	$(CPP) -I$(LIBPATH) -c $<

clean:
	@echo -en " CLEAN:\t";
	rm -f $(ALLPROGS) *.o

