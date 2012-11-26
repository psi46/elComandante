/*
 *	input.cpp	10-Oct-2007
 *	Dennis Terhorst <dennis.terhorst@rwth-aachen.de>
 */


#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "input.h"
#include "error.h"


// statics
int input::mode  = 0;
int input::lmLen = 0;
int input::lmRow = 0;
int input::lmCol = 0;
int input::linebuf_len = 0;
char input::linebuf[INP_LINEBUF_LEN];


// selectable properties
int input::getfd()     { return fd; }
int input::getchecks() { return CHK_READ; }

/**
 * read() stdin into the line buffer
 */
void input::fdr_call() {
	if ( fd < 0 ) return;
	char buf[80];	// FIXME: better use some reasonable constant here and below in case INP_LINE
	int len = 0;

	switch (mode) {
	case INP_CHAR:
		if ( (len = read(fd, &(linebuf[linebuf_len]), INP_LINEBUF_LEN - linebuf_len)) >= 0 ) {
			linebuf_len += len;
		} else {
			eperror("input.fdr_call read error");
			return;
		}
		break;
	case INP_LINE:
		if ( (len = read(fd, buf, 80 )) >= 0 ) {
			//fprintf(stderr, "buflen = %d\n", len);
			for (int i = 0; i < len; i++) {
			//	fprintf(stderr, "buf[%d] = %#02x\n", i, buf[i]);
				if ( ((buf[i]>32) && (buf[i]<127))) {						// printable
					if ( linebuf_len < INP_LINEBUF_LEN ) {
						linebuf[linebuf_len++] = buf[i];
						linebuf[linebuf_len] = 0;
						//fprintf(stderr, " -> append -%s-\n", linebuf);
					}
				} else if (strncmp(&(buf[i]), INP_EOL_STRING, strlen(INP_EOL_STRING)) == 0 ) {	// RETURN
						linebuf[linebuf_len++] = buf[i];
						linebuf[linebuf_len] = 0;
						//fprintf(stderr, " -> return -%s-\n", linebuf);
				} else if (buf[i] == INP_KEY_BACKSPACE) {					// BACKSPACE
					linebuf[--linebuf_len] = 0;
					//fprintf(stderr, " -> backsp -%s-\n", linebuf);
				} else if ( (i<79) && (buf[i] == INP_KEY_ESC) && (buf[i+1] == 0x00) ) {		// ESC only
					setCharMode();
					//fprintf(stderr, " -> ESCAPE -%s-\n", linebuf);
				}
			}
		} else {
			eperror("input.fdr_call read error");
			return;
		}
		break;
	default:
		fprintf(stderr, "%s:%d: unknown input mode\n",__FILE__,__LINE__);
	}

	input::redraw();
}

// constructor
input::input() {
	fd = -1;
	fd = open("/dev/stdin", O_RDONLY);
	if (fd<0) {
		eperror("ERROR: could not open /dev/stdin");
	}
	mode = INP_CHAR;
	linebuf_len = 0;
}

// destructor;
input::~input() {
	close(fd);
}


/**
 * Redraw current input
 *
 * This is needed especially for INP_LINE mode.
 */
void input::redraw() {

	switch (mode) {
	case INP_CHAR:
		break;
	case INP_LINE:
		page_t::setReversed();
		for (int i =0; i<lmLen; i++) {
			page_t::pprintf(lmRow,lmCol+i,"%c", (i<linebuf_len?linebuf[i]:' ') );
		}
		page_t::setNormal();
		break;
	default:
		fprintf(stderr, "%s:%d: unknown input mode\n", __FILE__,__LINE__);
	}
	return;
}


/**
 * check the line buffer for input and return data if available
 * \param buffer allocated buffer where data will be copied to
 * \param buflen length of allocated buffer. no more than buflen bytes will be written to buffer[]. On return
 *        buflen will be set to the number of remaining free bytes inside the buffer.
 * \return number of bytes read, zero indicates incomplete read (eg. in INP_LINE mode), -1 is returned on error.
 */
int input::readin(char* buffer, int* buflen)
{	
	if ( buffer == NULL ) return -1;
	if ( *buflen < 1 ) return -1;
	int ret = 0;
	int cplen=0;

	switch (mode) {
	case INP_CHAR:
		if ( linebuf_len > 0 ) {	// if linebuffer not empty

		// not buffer overrunning maximum number of chars to copy
			cplen = ((*buflen) > linebuf_len ? linebuf_len : (*buflen));

		// copy linebuf[] to buffer[]
			// fprintf(stderr, "will copy cplen=%d of linebuf_len=%d chars into *buflen=%d buffer\n", cplen, linebuf_len, *buflen);
			memcpy(buffer, linebuf, cplen);
			(*buflen) -= cplen;

			buffer[cplen] = 0;		// null termination actually not needed, i think
			// fprintf(stderr, "     copied buffer[]=-%s-\n", buffer);
			linebuf[linebuf_len]=0;		// null termination actually not needed, i think
			// fprintf(stderr, "     linebuf[]=-%s- premove\n", linebuf);

		// remove copied part from front of linebuf[]
			memmove(linebuf, &(linebuf[cplen]), linebuf_len - cplen);
			linebuf_len -= cplen;
			linebuf[linebuf_len]=0;		// null termination actually not needed, i think

			// fprintf(stderr, "     linebuf[]=-%s- postmove\n", linebuf);
			ret = cplen;
		}
		break;
	case INP_LINE:
		//fprintf(stderr, "readin search for EOL, linebuf_len=%d\n", linebuf_len);
		for (int i=0; i<linebuf_len; i++) {
			//fprintf(stderr, "readin   linebuf[%d] = %#02x ?= %#02x\n", i, linebuf[i], INP_EOL_STRING[0]);
			
			if ( strncmp( &(linebuf[i]), INP_EOL_STRING, strlen(INP_EOL_STRING) ) == 0) {	// if i is start of INP_EOL_STRING
				//fprintf(stderr, "readin found EOL\n");

			// not buffer overrunning maximum number of chars to copy
				cplen = ((*buflen) > i ? i : (*buflen));
				//fprintf(stderr, "readin will copy %d chars\n", cplen);

			// copy linebuf[] to buffer[]
				//fprintf(stderr, "will copy cplen=%d of i=%d chars into *buflen=%d buffer\n", cplen, i, *buflen);
				memcpy(buffer, linebuf, cplen);
				(*buflen) -= cplen;
				//fprintf(stderr, "readin buffer has %d chars left\n", *buflen);

				buffer[cplen] = 0;		// null termination actually not needed, i think
				//fprintf(stderr, "     copied buffer[]=-%s-\n", buffer);
				linebuf[linebuf_len]=0;			// null termination actually not needed, i think
				//fprintf(stderr, "     linebuf[]=-%s- premove\n", linebuf);

			// remove copied part from front of linebuf[]
				memmove(linebuf, &(linebuf[cplen]), linebuf_len - cplen);
				linebuf_len -= cplen;
				linebuf[linebuf_len]=0;
				//fprintf(stderr, "readin linebuf has %d chars left\n", linebuf_len);

				// fprintf(stderr, "     linebuf[]=-%s- postmove\n", linebuf);
				ret = cplen;
				//fprintf(stderr, "readin switching to char mode\n");
				setCharMode();
			}
		}
		break;
	default:
		eprintf("ERROR: class input has been set to an unknown mode. dropping input. setting mode=INP_CHAR.\n");
		linebuf_len = 0;
		mode = INP_CHAR;
	}
	//	int ret = read(fd, buffer, *buflen);
	//	if (ret>0) {
	//		*buflen -= ret;
	//		buffer[ret]=0;
	//	 }
	return ret;
}


/**
 * \brief set input mode to normal char mode
 *
 * discards remaining chars in \c linebuf[] by setting \c linebuf_len to zero.
 */
void input::setCharMode() {
	mode = INP_CHAR;
	linebuf_len = 0;	// discard remaining chars in linebuf
}


/**
 * \brief set line buffered input mode
 * 
 * This mode is intended for input fields of forms. Only printable chars (ascii 32-126) will appear
 * in the resulting string. command keys as backspace are parsed by input
 * and linebuf[] is changed accordingly. ESC will end line mode discarding the line buffer.
 *
 * \param row position of input field
 * \param col position of input field
 * \param len length of input field
 *
 * \todo input line mode should maybe parse some more keys: ins, del, cursors etc.
 */
void input::setLineMode(int row, int col, int len) {
	lmRow = row;
	lmCol = col;
	lmLen = len;
	mode = INP_LINE;
	return;
}

