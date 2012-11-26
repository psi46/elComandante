/*
 * keycodes.h
 * 	defines keycodes build with
 *	
	typedef union {
		int	i;
		char	c[sizeof(int)];
	} charint;
	int fd[0] = open("/dev/stdin", O_RDONLY);
	
	char input[MAX_INPUTLEN];
	int inlen;
	
	// READ FROM STDIN
	if ( (inlen=read(fd[0], &input, MAX_INPUTLEN)) < 0)
		scr->status("read error on stdin (fd%d)", fd[0]);

	// zero input buffer tail
	for (int i=inlen; i<MAX_INPUTLEN; i++)
		input[i]=0;
	
	c.c[0]=input[0];
	c.c[1]=input[1];
	c.c[2]=input[2];
	c.c[3]=input[3];

	switch (c.i) {
		case K_UP:
			[...]
	};
 *
 * DT 12/2005
 */
#ifndef KEYCODES_H
#define KEYCODES_H 1

#define KEY_UP		"\x1b\x5b\x41\x00"
#define KEY_DOWN	"\x1b\x5b\x42\x00"
#define KEY_RIGHT	"\x1b\x5b\x43\x00"
#define KEY_LEFT	"\x1b\x5b\x44\x00"

#define K_UP		0x00415b1b
#define K_DOWN		0x00425b1b
#define K_RIGHT		0x00435b1b
#define K_LEFT		0x00445b1b

#define K_PGUP		0x7e355b1b
#define K_PGDN		0x7e365b1b
#define K_HOME		0x00485b1b
#define K_END		0x00465b1b
#define K_DEL		0x7e335b1b
#define K_INS		0x7e325b1b

#define K_ESC		0x0000001b
#define K_TAB		0x00000009

#define K_F1		0x00504f1b
#define K_F2		0x00514f1b
#define K_F3		0x00524f1b
#define K_F4		0x00534f1b
// NOTE: these actually read as 5 byte sequence!
#define K_F5	0x35315b1b

#define K_BACKSPACE	0x0000007f
#define K_ENTER		0x0000000d
#define K_RETURN	K_ENTER

// CAUTION: some of these have special meanings!
#define K_CTRL_A	0x00000001
#define K_CTRL_B	0x00000002
#define K_CTRL_C	0x00000003
#define K_CTRL_D	0x00000004
#define K_CTRL_E	0x00000005
#define K_CTRL_F	0x00000006
#define K_CTRL_G	0x00000007
#define K_CTRL_H	0x00000008
#define K_CTRL_I	0x00000009
#define K_CTRL_J	0x0000000a
#define K_CTRL_K	0x0000000b
#define K_CTRL_L	0x0000000c
#define K_CTRL_M	0x0000000d
#define K_CTRL_N	0x0000000e
#define K_CTRL_O	0x0000000f
#define K_CTRL_P	0x00000010
#define K_CTRL_Q	0x00000011
#define K_CTRL_R	0x00000012
#define K_CTRL_S	0x00000013
#define K_CTRL_T	0x00000014
#define K_CTRL_U	0x00000015
#define K_CTRL_V	0x00000016
#define K_CTRL_W	0x00000017
#define K_CTRL_X	0x00000018
#define K_CTRL_Y	0x00000019
#define K_CTRL_Z	0x0000001a

#define K_ALT_A		0x0000611b
#define K_ALT_B		0x0000621b
#define K_ALT_C		0x0000631b
#define K_ALT_D		0x0000641b
#define K_ALT_E		0x0000651b
#define K_ALT_F		0x0000661b
#define K_ALT_G		0x0000671b
#define K_ALT_H		0x0000681b
#define K_ALT_I		0x0000691b
#define K_ALT_J		0x00006a1b
#define K_ALT_K		0x00006b1b
#define K_ALT_L		0x00006c1b
#define K_ALT_M		0x00006d1b
#define K_ALT_N		0x00006e1b
#define K_ALT_O		0x00006f1b
#define K_ALT_P		0x0000701b
#define K_ALT_Q		0x0000711b
#define K_ALT_R		0x0000721b
#define K_ALT_S		0x0000731b
#define K_ALT_T		0x0000741b
#define K_ALT_U		0x0000751b
#define K_ALT_V		0x0000761b
#define K_ALT_W		0x0000771b
#define K_ALT_X		0x0000781b
#define K_ALT_Y		0x0000791b
#define K_ALT_Z		0x00007a1b

#define K_ALT_0		0x0000301b
#define K_ALT_1		0x0000311b
#define K_ALT_2		0x0000321b
#define K_ALT_3		0x0000331b
#define K_ALT_4		0x0000341b
#define K_ALT_5		0x0000351b
#define K_ALT_6		0x0000361b
#define K_ALT_7		0x0000371b
#define K_ALT_8		0x0000381b
#define K_ALT_9		0x0000391b

#endif
