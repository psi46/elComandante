magboltz: magboltz.o
	g77 -o magboltz magboltz.o -L/cern/pro/lib -lpacklib -lmathlib -lkernlib
magboltz.o: magboltz.f
	g77 -o magboltz.o -fno-underscoring -w -c magboltz.f
clean:
	rm -f magboltz magboltz.o
