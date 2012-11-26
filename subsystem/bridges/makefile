
SUBDIRS=subserver
ifneq (,$(findstring WITH_IRC,$(BUILD_OPTIONS)))
SUBDIRS+=irc
endif
ifneq (,$(findstring WITH_MYSQL,$(BUILD_OPTIONS)))
SUBDIRS+=mysql
endif
ifneq (,$(findstring WITH_CURL,$(BUILD_OPTIONS)))
SUBDIRS+=curl
endif

.PHONY:	all clean install bin deb $(SUBDIRS)

all:	$(SUBDIRS)
	@echo " DONE";

clean:	$(SUBDIRS)
	@echo " CLEANED";

install:
bin:
deb:

$(SUBDIRS):
	@cd $@;	$(MAKE) $(MAKECMDGOALS);

