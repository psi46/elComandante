/*
 *	input.h		0-Oct-2007
 *	Dennis Terhorst <dennis.terhorst@rwth-aachen.de>
 */
#ifndef INPUT_H
#define INPUT_H

#include "selectable.h"

#define INP_LINEBUF_LEN	256

/// set this to "\r\n" if neccessary
#define INP_EOL_STRING	"\x0D"
#define INP_KEY_BACKSPACE	0x7f
#define INP_KEY_ESC		0x1b

#define INP_KEY_UP	"\x1b\x5b\x41\x00"
#define INP_KEY_DOWN	"\x1b\x5b\x42\x00"
#define INP_KEY_RIGHT	"\x1b\x5b\x43\x00"
#define INP_KEY_LEFT	"\x1b\x5b\x44\x00"

#define INP_CHAR	1
#define INP_LINE	2
#define INP_NUMBER	3
#define INP_HEX		4
#define INP_BINARY	9

/**
 * \brief stdin wrapper
 *
 * This class is derived from class \ref selectable and thus is capable of being
 * handled with multiplexed i/o.
 */
class input : public selectable {
private:
	int fd;

	static int mode;	///< current input mode
	static int lmRow;	///< line mode row
	static int lmCol;	///< line mode column
	static int lmLen;	///< line mode length

	static char linebuf[INP_LINEBUF_LEN];		///< buffer for INP_LINE
	static int linebuf_len;			///< valid data length in linebuf

        // selectable properties
        int getfd()    ;
        int getchecks();

public:
	void fdr_call();

	input();

	virtual ~input();

	static void redraw();

	int readin(char* buffer, int* buflen);

	static void setLineMode(int row, int col, int len);
	static void setCharMode();
};

#endif
