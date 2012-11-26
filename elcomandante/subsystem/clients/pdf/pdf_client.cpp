
#include "pdf_client.h"
#include <string.h>
/**
 * Construktor 
 * \param filename filename of the pdf-file what should be created
 */
pdf_client::pdf_client(char* filename){
	pdf = PDF_new();
	pdffilename = filename;
	if (pdf == NULL) {
		fprintf(stderr, "Error: cannot open output file %s.\n", pdffilename);
		exit(1);
	}
	width = a4_width;
	height = a4_height;
	margin = 20;    
	fontname = "Courier";
	fontsize = 10.0;
	encoding = "host";
}

pdf_client::~pdf_client(){
	if (y != height - margin){
		PDF_end_page_ext(pdf, "");
	}
	PDF_end_document(pdf, "");
	PDF_delete(pdf);
}
/**
 * initialize new document
 */
void pdf_client::init(){
	PDF_begin_document(pdf, pdffilename, 0, "");
	PDF_set_info(pdf, "Title", "notitle");
	PDF_set_info(pdf, "Creator", "subserver");
	x = margin;
	y = height - margin;
}
/**
 * close document but keep pdf creator
 */
void pdf_client::close(){
	if (y != height - margin){
		PDF_end_page_ext(pdf, "");
	}
	PDF_end_document(pdf, "");
}

void pdf_client::pprintf(char* line){
	if(line[0] == PDF_FF) {
		if (y == height - margin){
			PDF_begin_page_ext(pdf, width, height, "");
		}
		PDF_end_page_ext(pdf, "");
		y = height - margin;
		return;
	}

	if (line[0] != '\0' && line[strlen(line) - 1] == PDF_NL)
		line[strlen(line) - 1] = '\0';	/* remove newline character */

	if (y < margin) {		/* page break necessary? */
		y = height - margin;
		PDF_end_page_ext(pdf, "");
	}

	if (y == height - margin) {
		PDF_begin_page_ext(pdf, width, height, "");
		font = PDF_load_font(pdf, fontname, 0, encoding, "");
		PDF_setfont(pdf, font, fontsize);
		PDF_set_text_pos(pdf, x, y);
		y -= fontsize;
	}

	PDF_continue_text(pdf, line);
	y -= fontsize;
}
