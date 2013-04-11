.PHONY: all doc pdf

all: doc

doc:
	cd .. && doxygen doc/Doxyfile

pdf: doc
	make -C latex
