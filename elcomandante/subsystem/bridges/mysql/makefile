LIBPATH=../..

CC=/usr/bin/g++

CCFLAGS+=-I$(LIBPATH)
CCFLAGS+=`mysql_config --cflags`
LDFLAGS+=-lsubsystem -L$(LIBPATH)
LDFLAGS+=`mysql_config --libs`

# Program opject dependencies
CLIENT_MYSQL_OBJ=mysql_client.o

ALLPROGS=client_mysql

.PHONY:	all clean

all:	$(ALLPROGS) 
	@echo " DONE";

client_mysql:	client_mysql.o $(CLIENT_MYSQL_OBJ)
	@echo -en " LD $@:\t";
	$(CC) -o $@ $@.o $(CLIENT_MYSQL_OBJ) $(LDFLAGS)

%.o:	%.c | %.h
	@echo -en " cc $<:\t";
	$(CC) $(CCFLAGS) -c $<

%.o:	%.cpp | %.h
	@echo -en " CC $<:\t";
	$(CC) $(CCFLAGS) -c $<

clean:
	@echo -en " CLEAN:\t";
	rm -f $(ALLPROGS) *.o


