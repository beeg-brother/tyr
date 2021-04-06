#include <iostream>
#include <curses.h>

void curses_setup(){
    // initialize curses
    initscr();
    // start color system
    //start_color();
    //use_default_colors();
    // refresh screen
    refresh();
    cbreak();
    noecho();
    keypad(stdscr, true);
}


int main() {
	curses_setup();

    wattrset(stdscr,COLOR_PAIR(1));
    mvaddnstr(1, 0, "0123456789", 10);
    move(1,0);
    clrtoeol();

    refresh();
    doupdate();

    int a = getch();

	endwin();
	return 0;
}
