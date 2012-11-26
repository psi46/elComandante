#CFLAGS=-Wall -DSUB_CLIENT 0
CFLAGS=-Wall -I$$HOME/lib
LDFLAGS=-Wall -lncurses -lsubsystem -L$$HOME/lib
CC=g++
ECHO=/bin/echo

PROG=subview
#OBJ=serial.o flowproto.o selectable.o fbnode.o flowbus.o input.o parmchain.o procchain.o request.o errorpage.o nodepage_flow.o page_t.o fifo.o masterpage.o nodepage_press.o parameter_defs.o flowbus_page.o mixture.o valve.o valveblock.o valveblock_page.o tree.o strbuf.o conf.o config.o main.o

.PHONY:	all clean doc backup check ipcrm
all:	$(PROG)
	@$(ECHO) " DONE"

#$(PROG): selectable.o selectable_sclient.o input.o errorpage.o page_t.o  masterpage.o tree.o strbuf.o conf.o config.o main.o aboinfo.o
subview.o:	miniparser.h
ncurses_textbox.cpp: ncurses_screen_tabstop.h

$(PROG):	$(PROG).o
$(PROG):	ncurses_element.o ncurses_screen.o ncurses_keyboard.o
$(PROG):	ncurses_textbox.o ncurses_label.o
$(PROG):	abo_info.o abo_display.o abo_info_manager.o
#$(PROG):	 sclient provieded by lsubsystem
$(PROG):	selectable.o selectable_sclient.o selectable_ncurses_keyboard.o


$(PROG):
	@$(ECHO) -en " LD $@:\t"
	g++ $^ $(LDFLAGS) -o $@


%.o:	%.cpp  %.h
	@$(ECHO) -en " CC $@:\t"
	g++ $(CFLAGS) -c $<

%.o:	%.cpp
	@$(ECHO) -en " CC $@:\t"
	g++ $(CFLAGS) -c $<

%.o:	%.c  %.h
	@$(ECHO) -en " CC $@:\t"
	gcc $(CFLAGS) -c $<

ipcrm:
	@$(ECHO) -en " Q  removing message queues: "
	for QID in `ipcs -q | grep "0x0" | cut -f 2 -d" "`; do ipcrm -q $$QID; done

clean:
	@$(ECHO) -en " CLEAN ALL:\t"
	rm -f *.o $(PROG)

doc:
	@$(ECHO) -en " DOC:\t"
	doxygen >/dev/null

#DATE=$(shell date '+%Y%m%d-%s')
#backup:	clean
#	cd ..; tar -cvjf $$HOME/flowbus-software$(DATE).tar.bz2 software; cd software

check:
	@$(ECHO) -e "\nConstructors/Destructors in .cpp files:"
	@for file in *.h; do f=`$(ECHO) $$file|cut -f1 -d.`; egrep "$$f::.?$$f" $$f.cpp; done
	@$(ECHO) -e "\nConstructors/Destructors in .h files:"
	@for file in *.h; do f=`$(ECHO) $$file|cut -f1 -d.`; egrep "^.?(.?|.*virtual.*)$$f" $$f.h; done
	@$(ECHO) -e "\nIncludes in .h files"
	@egrep "include *\"" *.h
	@$(ECHO) -e "\nIncludes in .cpp files"
	@egrep "include *\"" *.cpp
	@$(ECHO) -e "\nTODOs in .cpp and .h files"
	@egrep "TODO|not implemented" *.h *.cpp
	@$(ECHO) -e "\nFIXMEs in .cpp and .h files"
	@egrep "FIXME" *.h *.cpp

