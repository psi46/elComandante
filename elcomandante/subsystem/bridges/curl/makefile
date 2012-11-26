SUBDIRS=rwth-weather speiseplan tek-data

.PHONY:	all clean $(SUBDIRS)

all:	$(SUBDIRS)
	@echo " DONE";

clean:	$(SUBDIRS)
	@echo " CLEANED";

$(SUBDIRS):
	@cd $@;	$(MAKE) $(MAKECMDGOALS);

