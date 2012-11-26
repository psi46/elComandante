

LIBNAME=libtest.so
LIBOBJECTS=libtest.o
CC=gcc
TESTPROG=demo_dynamic

.PHONY: all clean
all:	$(TESTPROG)
# $(LIBNAME) 

$(TESTPROG): $(TESTPROG).o
	@echo -en " [LD] "
	$(CC) -g -o $(TESTPROG) $(TESTPROG).o -ldl -lc

$(TESTPROG).o:	$(TESTPROG).c
	@echo -en " [CC] "
	$(CC) -Wall -g -c $(TESTPROG).c

$(LIBNAME): $(LIBOBJECTS)
	@echo -en " [LD] "
	# Create shared library.
	# Use -lc to link it against C library, since libhello
	# depends on the C library.
	$(CC) -g -shared -Wl,-soname,$(LIBNAME).0 \
	    -o $(LIBNAME).0.0 $(LIBOBJECTS) -lc

libtest.o: libtest.c
	@echo -en " [CC] "
	$(CC) -fPIC -Wall -g -c $<

clean:
	@echo -en " [CLEAN] "
	rm -f $(TESTPROG){,.o}
