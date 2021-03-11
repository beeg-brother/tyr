#include <iostream>
#include <curses.h>

void curses_setup(){
    // initialize curses
    initscr();
    // start color system
    start_color();
    use_default_colors();
    // refresh screen
    refresh();
    cbreak();
    noecho();
    keypad(stdscr, true);
}


int main() {
	curses_setup();

	init_pair(1, 40, -1);
	init_pair(2, 21, -1);
	init_pair(3, 255, -1);
	init_pair(4, 154, -1);
	init_pair(5, 226, -1);
	init_pair(6, 245, -1);
	init_pair(7, 255, -1);

    wattrset(stdscr,COLOR_PAIR(1));
    mvaddnstr(1, 0, "borderFocused", 30);

    wattrset(stdscr, COLOR_PAIR(2));
    mvaddnstr(2, 0, "borderUnfocused", 30);

    wattrset(stdscr,COLOR_PAIR(3));
    mvaddnstr(3, 0, "text", 30);

    wattrset(stdscr,COLOR_PAIR(4));
    mvaddnstr(4, 0, "directories", 30);

    wattrset(stdscr,COLOR_PAIR(5));
    mvaddnstr(5, 0, "files", 30);

    wattrset(stdscr,COLOR_PAIR(6));
    mvaddnstr(6, 0, "lineNumbers", 30);

    wattrset(stdscr,COLOR_PAIR(7));
    mvaddnstr(7, 0, "selectedItem", 30);

    refresh();
    doupdate();


    int a = getch();


	endwin();
	return 0;
}

/*
borderFocused:40
borderUnfocused:21
text:255
directories:154
files:226
lineNumbers:245
selectedItem:255
 */