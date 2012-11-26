

sub-lg: sub-lg.cpp
	g++ -I../.. $^ -L../.. -lsubsystem -o $@

.PHONY: install
install: sub-lg
	cp -v sub-lg ~/bin/
