.PHONY: all doc pdf

all: doc

doc:
	doxygen Doxyfile

pdf: doc
	make -C latex
