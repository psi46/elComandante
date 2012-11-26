#
# Subsystem Makefile
#	Dennis Terhorst
#	Sep 11 2008
#
#
# Give paths for installation here:
LIB_INSTALL_PATH=$(HOME)/lib
INC_INSTALL_PATH=$(LIB_INSTALL_PATH)/subsystem
BIN_INSTALL_PATH=$(HOME)/bin

# Bridges: BUILD_OPTIONS =
# 	WITH_MYSQL
# 	WITH_CURL
# 	WITH_IRC
#BUILD_OPTIONS = WITH_CURL
#               WITH_CURL WITH_IRC WITH_MYSQL
# Clients: 
# 	WITH_SUBLPT
# 	WITH_PDF
# 	WITH_CONFIGURE
# 	WITH_SUBSCRIPT
# 	WITH_WATCHDOG
# 	WITH_AGGREGATOR
# 	WITH_THPC_SPLIT
#BUILD_OPTIONS += WITH_THPC_SPLIT
#BUILD_OPTIONS += WITH_CONFIGURE WITH_WATCHDOG WITH_AGGREGATOR
#BUILD_OPTIONS += WITH_SUBSCRIPT 
export BUILD_OPTIONS

##############################################################################
### You do not want to change anything below this line!
###    (unless you know what you are doing...)
###

# library name
LIB=libsubsystem.a

# objects to be included in the library
LIBOBJECTS=subserver.o clientinfo.o abo.o daemon.o logfile_t.o
LIBOBJECTS+=sclient.o packet_t.o cmdint/convert.o 
LIBOBJECTS+=CPU_info.o subClientHandler.o 
LIBOBJECTS+=selectable.o selectable_sclient.o
UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	OS="LINUX
	OSDEV=-D'OS="Linux"'
endif
ifeq ($(UNAME), Solaris)
# do something Solaris-y
	OS="SOLARIS"
	OSDEV=-D'OS="SOLARIS"'
endif
ifeq ($(UNAME), Darwin)
# do something Solaris-y
	OS="OSX"
	OSDEV=-D'MAC_OS="OSX"'
	
endif
# compile and link options
ifeq ($(OS),Windows_NT)
    CC=g++.exe
    CFLAGS=-c $(OSDEV)
    CCFLAGS=$(CFLAGS)
    LDFLAGS=-Lc:/Programme/MinGW/lib -lwsock32 -lws2_32 $(OSDEV)
else
    CC=/usr/bin/g++
    CFLAGS=-c $(OSDEV)
    CFLAGS+=-DDEFAULT_LOGDIR=\"$(HOME)/DATA\"
    CCFLAGS=$(CFLAGS)
    LDFLAGS=$(OSDEV) 
    ECHO=$(shell which echo)
endif


ALLPROGS:=server txclient rxclient client
ALLPROGS2:=subserver txclient rxclient client
#subcalibrate subprotocol client_logchange
PRE_SUBDIRS:=cmdint
POST_SUBDIRS:=bridges clients


.PHONY:	all clean lib doc checkenv install bin $(PRE_SUBDIRS) $(POST_SUBDIRS) deb

all:	$(PRE_SUBDIRS) $(ALLPROGS) lib $(POST_SUBDIRS) checkenv
	mv server subserver
	@$(ECHO) " DONE ALL";

checkenv:
	@$(ECHO) " CHECK:	SUBSERVER = \"$$SUBSERVER\"";
	@if [ -z "$$SUBSERVER" ]; then $(ECHO) -e ' NOTE:\tSUBSERVER not set!'; $(ECHO) -e "*** please type something like:\n\texport SUBSERVER=\"127.0.0.1:12334\""; fi

$(PRE_SUBDIRS) $(POST_SUBDIRS):
	@cd $@;	$(MAKE) $(MAKECMDGOALS);

# Program opject dependencies
DEFAULT_OBJS:=	packet_t.o
server:		subserver.o clientinfo.o abo.o daemon.o logfile_t.o CPU_info.o 
client_logchange: sclient.o cmdint/convert.o daemon.o
client:		sclient.o cmdint/convert.o daemon.o
subcalibrate:	sclient.o transfer_function_t.o
subprotocol:	sclient.o
txclient:	sclient.o
rxclient:	sclient.o
ifneq (, $(findstring clean,$MAKECMDGOALS))
$(POST_SUBDIRS):	lib
endif

.SECONDEXPANSION:
$(ALLPROGS):	$$@.o $(DEFAULT_OBJS)
#%:	%.o 
ifeq ($(OS), Windows_NT)
	@$(ECHO) -n "[Windows_NT] "
	$(CC)  $@.o $(CLIENT_OBJ) $(LDFLAGS) -o $@.exe
else
	@$(ECHO) -en " LD $@:\t";
	$(CC) -o $@ $^ $(LDFLAGS)
endif

%.o:	%.c | %.h
	@$(ECHO) -en " cc $@:\t";
	$(CC) $(CFLAGS) $<

%.o:	%.cpp | %.h
	@$(ECHO) -en " CC $@:\t";
	$(CC) $(CCFLAGS) $< -o $@

##############################################################################

clean:	$(PRE_SUBDIRS) $(POST_SUBDIRS)
	@$(ECHO) -en " CLEAN ALL:\t";
ifneq ($(OS), Windows_NT)
	rm -f $(ALLPROGS) *.o $(LIB); rm -rf deb
else
	del *.o
endif
	@$(ECHO) -e " CLEANED ALL\n";

lib:	$(LIBOBJECTS)
	@#$(ECHO) -e " CP inclued files (local)"
	@#mkdir -p subsystem/
	@#cp -Luf --parents `find . -path ./subsystem -prune -o -name "*.h" | grep -v "./subsystem"` subsystem
	@$(ECHO) -e " AR $(LIB)"
	@ar rs $(LIB) $(LIBOBJECTS)
	@$(ECHO) -e " LS $(LIB) now contains"
	@ar tv $(LIB)

install:	lib
	@$(ECHO) -e " CP -rf $(LIB_INSTALL_PATH)/$(LIB)"
	@$(ECHO) make Dir: " $(LIB_INSTALL_PATH)" 
	@mkdir -p $(LIB_INSTALL_PATH)
	@$(ECHO) Copy LIB
	@cp -rfv $(LIB) $(LIB_INSTALL_PATH)
	@$(ECHO) " CP inclued files"
	@mkdir -p $(INC_INSTALL_PATH)
	@$(ECHO) cp to "$(INC_INSTALL_PATH)"
	@cp -Lvf `find . -name "*.h"` $(INC_INSTALL_PATH)
	@INCFILES=`find . \( -path "./clients"\
			-or -path "./documentation"\
			-or -path "./subsystem*"\
			-or -path "./qt4" \) -prune -o -name "*.h" -a -print`;
	@$(ECHO) COPY INCLUDE FILES: "$(INCFILES) $$INCFILES"
	@$(ECHO) $$INCFILES
	#@cp -Lfvr $$INCFILES $(INC_INSTALL_PATH)
	@$(ECHO) -e " INSTALL ok."

bin:	all
	@$(ECHO) -e "copy binaries to $(BIN_INSTALL_PATH)"
	@mkdir -p $(BIN_INSTALL_PATH)
	@cp -v $(ALLPROGS2) $(BIN_INSTALL_PATH)

deb:	all
	@$(ECHO) " DEB: Debian Package"
	@$(ECHO) "    creating directory structure"
	@mkdir -p deb/{DEBIAN,usr/{bin,include}/subserver,usr/lib}
	@$(ECHO) -e "    copy files in deb structure"
	#copy binaries into /usr/bin/subserver
	@cp -v DEBIAN_control deb/DEBIAN/control
	@cp -v $(ALLPROGS2) ./deb/usr/bin/subserver
	#copy include files into /usr/include/subserver
	@cp -Luvr --parents `find . -name "*.h"` ./deb/usr/include/subserver
	#copy lib into /usr/lib
	@cp -v $(LIB) ./deb/usr/lib
	@$(ECHO) -e "    make package"
	dpkg -b deb subserver-0.0.7-i386.deb
	@$(ECHO) -e "    clean up"
	@rm -rf deb
	@$(ECHO) " DEB: DONE"
doc:
	@$(ECHO) -en " DOC:\t"
	doxygen >/dev/null
	@$(ECHO) -e "\n DOC DONE\n"



