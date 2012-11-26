/**
 * \file subview.cpp
 * \author Dennis Terhorst
 * \date Thu Aug 20 17:53:30 CEST 2009
 */

//#include "ncurses_screen.h"
#include "ncurses_textbox.h"
#include "ncurses_label.h"
#include "selectable_sclient.h"
#include "selectable_ncurses_keyboard.h"
#include <errno.h>
#include <libgen.h>	// basename()

#ifdef WITH_YACC
// FIXME: this is for the start only...
#include "parser.h"
FILE* yyin=stdin;
int yyparse() { return 0; }
#endif

////////////////////////////////////////////////////////////////////////////////
#include "miniparser.h"

////////////////////////////////////////////////////////////////////////////////
#include "abo_info.h"
#include "abo_info_manager.h"
#include "abo_display.h"
////////////////////////////////////////////////////////////////////////////////


typedef enum {TEXTBOX, ABOLIST} keyboard_mode_t;
#include <time.h>

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////  G L O B A L  //
#include "global.h"
GLOBAL_VARS_T global;
ncurses wout(&screen,screen.height()/2, screen.width(), screen.height()/2, 0);
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

	// create default output window
	//ncurses wout(&screen,screen.height()/2, screen.width(), screen.height()/2, 0);
	wout.scrolling(true);
	wout << "\nInitializing...\n";

	// register keyboard input
	wout << "setup keyboard input... ";
	ncurses_keyboard_selectable keyboard;
	keyboard_mode_t keyboard_mode = TEXTBOX;
	wout << "ok\n";

	// setup connection to subsystem
	wout << "setup subsystem client... ";
	sclient_selectable subclient;
	if ( subclient.isOK() ) {
		subclient.setid(basename(argv[0]));
		wout << "ok\n";
		global.meptr = &subclient;
	} else {
		wout << BOLD << "FAILED! " << NOBOLD << "Aborting.\n";
		getch();
		return -1;
	}

	// Window elements
	wout << "setup window elements... ";
	char chartime[12];
	ncurses win_time(&screen, 1,screen.width(),0,0);	// create status bar window
	snprintf(chartime, 12, "%ld", time(NULL));
	label lbl_time(&win_time,0,0, chartime);
	win_time.autoclear();

	// sclient output window
	abo_display abo_list(&screen, screen.height()/2-1,screen.width(),1,0);	// create sclient output window
/*	ncurses win_sclientrx(&screen, screen.height()/2-1,screen.width(),1,0);	// create sclient output window
	win_sclientrx.clear();
	win_sclientrx << "SCLIENT OUTPUT WINDOW\n";
	win_sclientrx.scrolling();
	win_sclientrx.clear();*/
	wout << "ok\n";

	ncurses window(&screen, 1,80,screen.height()-1, 1);	// create command line window
	textbox txt_command(&window, 0,10, screen.width()-11);
	//window.set_tabstop(&txt_command);
	label lbl_mode(&window,0,0, "command:");
	window.autoclear();

	wout << BOLD << "ready.\n" << NOBOLD;

	int ch;
	global.wantexit=0;
	while (! global.wantexit) {
		//wout << ".";
		switch (keyboard_mode) {
		case TEXTBOX:
			lbl_mode.set_text("command:");
			// txt_command.place_cursor(); FIXME FIXME this is a bad hack!
			move(screen.height()-1,10+txt_command.get_text().size()+2);
			break;
		case ABOLIST:
			lbl_mode.set_text("ABOLIST");
			abo_list.place_cursor();
			break;
		}
		screen.redraw();
		switch ( selectable::run(1,0) ) {
		case -1:	// error
			if (errno == EINTR) continue;
			wout << "an errno " << errno << " has occoured in select() call";
			break;
		case 0:		// timeout
			snprintf(chartime, 12, "%ld", time(NULL));
			lbl_time.set_text(chartime);
			break;
		default:	// ready
			if ( keyboard.isready(CHK_READ) ) {
				ch = keyboard.getchar();
				switch ( keyboard_mode ) {
				case TEXTBOX:
					switch ( ch ) {
					case 10:
					case KEY_ENTER:
						parse(txt_command.get_text());
						txt_command.set_text("");	// clear command line
						break;
/*					case 'q':
						global.wantexit++;
						break;
					case KEY_F(1):
						win_sclientrx << time(NULL) <<  ": HERE\n";
						break;
					case KEY_F(2):
						global.meptr->subscribe("/test");
						wout << "subscribed /test\n";
						break;*/
					case KEY_F(1):
						keyboard_mode=ABOLIST;
						break;
					case KEY_NPAGE:
						wout << "NPAGE\n";
						break;
					case KEY_PPAGE:
						wout << "PPAGE\n";
						break;
					default:
						//wout << "keyboard input " << ch << "\n";
						txt_command.input(ch);
					} // end switch ch
					break;
				case ABOLIST:
					switch ( ch ) {
					case KEY_F(1):
						keyboard_mode=TEXTBOX;
					default:
						wout << "keyboard input " << ch << "\n";
						//txt_command.input(ch);
					} // end switch ch
					break;
					
				default:
					global.wantexit++;
				} // end switch keyboard_mode
			} // end if keyboard read ready
			if ( subclient.isready(CHK_READ) ) {
				//wout << "subclient input\n";
				packet_t packet;
				if ( global.meptr->recvpacket(packet) > 0 ) {	// FIXME: some fancy display here!
					abo_list.display(packet);
				}
			}
		} // end switch selectable
	} // end while (! global.wantexit)

	wout << BOLD << "good bye!\n";
	screen.redraw();
	usleep(700000);
}
