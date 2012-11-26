
#include "ncurses_screen.h"

int main(void) {
	screen << "hallo welt\n";

	WINDOW* window = derwin(stdscr, 5,6,7,8);
	wprintw(window, "hallo window");
	wrefresh(window);
	//ncurses window(&screen, 5,6,7,8);
	//window << "hallo window";
	//window.redraw();
	wrefresh(stdscr);
	getch();
	return 0;
}
