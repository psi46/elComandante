/**
 * \file main.cpp
 * ncurses-based subsystem packet subscription client
 *
 * \author Dennis Terhorst
 * \date Fri Jul 24 12:57:36 CEST 2009
 * modified version of flowbus ncurses-based frontend from
 *	main.cpp	10-Oct-2007
 *	Dennis Terhorst <dennis.terhorst@rwth-aachen.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
	// for open
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

#include <stdlib.h>     // atexit()
#include <signal.h>     // signal()
#include <time.h>	// time()

// main instanciated pages:
#include "input.h"	// selectable stdin
#include "error.h"	// with USE_ERRORPAGE
#include "masterpage.h" // MASTERPAGE

#include "config.h"	// read .ini-like config files

#include "selectable_sclient.h"

#define KEY_PGUP	"\x1b\x5b\x35\x7e"
#define KEY_PGDN	"\x1b\x5b\x36\x7e"
#define KEY_ESC		"\x1b\x00"

#define MAX_PAGES	16


void sigint_handler(int sig) {
        eprintf("received SIGINT\n");
}

sclient* meptr;

#include <map>
#include <string>
using namespace std;
#include "aboinfo.h"

int main() {
        signal(SIGINT, sigint_handler);

	// INIT I/O
	input inp;
//	sclient_selectable me;
//	if ( !me.isOK() ) {
//		fprintf(stderr, "could not init sclient!\n");
//		return 1;
//	}
//	meptr = &me;

	// INIT NCURSES
	page_t::init();		// ncurses init
	page_t::cursor_off();
	errorpage*  ep = new errorpage();
	masterpage* mp = new masterpage();
	mp->setTitle("MASTERPAGE");


	// INIT CONFIG
//	char config_filename[] = "gas_system.conf";
//	config * conf = new config(config_filename);
//	eprintf("Config file found and read in");
	int wasTimeout;

	while (1) {

		wasTimeout=0;								// select()
		page_t::bouncer();
		page_t::redraw();
		if ( selectable::run(0,250000) == 0) { // timeout
			wasTimeout=1;
		}
		usleep(100); // wait for multibyte chars to aggregate

		/*	my_vb->update();
			page_t::redraw();
					fprintf(stderr, "MAINPAGE\n");
					page_t::setActivepage(mp);	// set mainpage
		*/
												// process select() results
//		if ( me.isready(CHK_READ) )	{
//			// bus.fdw_call();
//		}
		if ( inp.isready(CHK_READ) )	{						// << stdin
			inp.fdr_call(); // let input process the received chars

			char buffer[80];
			int buflen=80;
			int ret = inp.readin(buffer, &buflen);	// check if a complete input is available
			if ( ret < 0 ) {
				eprintf("main: input read returned %d\n", ret);
			}

			// parse command line commands
			if ( ret > 0 ) {		// ------------------------- PARSE
				if      (strncmp(buffer, "q", 1) == 0) { break; }
				else if (strncmp(buffer, KEY_PGDN, strlen(KEY_PGDN)) == 0) { page_t::pagedown(); }
				else if (strncmp(buffer, KEY_PGUP, strlen(KEY_PGUP)) == 0) { page_t::pageup(); }
				else if (strncmp(buffer, "p", 1) == 0) { page_t::pageup(); }
				else if (strncmp(buffer, "n", 1) == 0) { page_t::pagedown(); }
				// ESC ist doof                        else if (buffer[0] == KEY_ESC[0] && buffer[1] == 0) { break; }
				else {
				eprintf("passing unparsed key to activepage. (%02X %02X %02X %02X %02X)\n",
					buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
				//page_t::getActivepage()->input(buffer, ret);
				}
		
			} // end if ( ret > 0 )

		} // end if (inp.isready(CHK_READ))
	} // end main loop

//	delete(conf);
	delete(mp);
	delete(ep);
//	page_t::cursor_on();
//	page_t::uninit();
	fprintf(stderr, "main: normal exit.\n");

	return 0;
}

