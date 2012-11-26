/**
 * \file pdf_client.h
 * \author Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 * For use with free avaiable PDFlib-Lite-7.0.3 get it from
 * http://www.pdflib.com/de/download/pdflib-familie/pdflib-lite/
 */

#ifndef PDF_CLIENT_H
#define PDF_CLIENT_H

#include <pdflib.h>
#include <stdlib.h>

#define PDF_NL '\n'
#define PDF_FF '\f'

class pdf_client {
	private:
		PDF *pdf;
		int opt;
		int font;
		char *fontname;
		char *pdffilename;
		char *encoding;
		double fontsize;
		double x, y, width, height, margin;

	public:
		pdf_client(char* filename);
		~pdf_client();
		void init();
		void pprintf(char* line);
		void newpage();
		void close();
};
#endif //ndef PDF_CLIENT_H
