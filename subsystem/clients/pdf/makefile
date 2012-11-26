CC=/usr/bin/g++
LIBPATH=../..
LDFLAGS+=-lsubsystem -L$(LIBPATH)
LDFLAGS+=`pdflib-config --ldflags`
CCFLAGS+=-I$(LIBPATH)
CCFLAGS+=`pdflib-config --cflags`

# Program opject dependencies
CLIENT_PDF_OBJ=pdf_client.o

ALLPROGS=client_pdf

.PHONY:	all clean

all:	$(ALLPROGS) 
	@echo " DONE";

client_pdf:	client_pdf.o $(CLIENT_PDF_OBJ)
	@echo -en " LD $@:\t";
	$(CC) -o $@ $@.o $(CLIENT_PDF_OBJ) $(LDFLAGS)

%.o:	%.c | %.h
	@echo -en " cc $<:\t";
	$(CC) $(CCFLAGS) -c $<

%.o:	%.cpp | %.h
	@echo -en " CC $<:\t";
	$(CC) $(CCFLAGS) -c $<

clean:
	@echo -en " CLEAN:\t";
	rm -f $(ALLPROGS) *.o
