SUBDIRS=

SUBDIRS+=subprotocol
ifneq (,$(findstring WITH_SUBCALIBRATE,$(BUILD_OPTIONS)))
SUBDIRS+=subcalibrate
endif
ifneq (,$(findstring WITH_THPC_SPLIT,$(BUILD_OPTIONS)))
SUBDIRS+=thpc_split
endif
ifneq (,$(findstring WITH_AGGREGATOR,$(BUILD_OPTIONS)))
SUBDIRS+=aggregator
endif
ifneq (,$(findstring WITH_CONFIGURE,$(BUILD_OPTIONS)))
SUBDIRS+=configure
endif
ifneq (,$(findstring WITH_WATCHDOG,$(BUILD_OPTIONS)))
SUBDIRS+=watchdog
endif
ifneq (,$(findstring WITH_SUBSCRIPT,$(BUILD_OPTIONS)))
SUBDIRS+=subscript
endif
ifneq (,$(findstring WITH_PDF,$(BUILD_OPTIONS)))
SUBDIRS+=pdf
endif
ifneq (,$(findstring WITH_SUBLPT,$(BUILD_OPTIONS)))
SUBDIRS+=sublpt
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
