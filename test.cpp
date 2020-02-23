#include <iostream>
#include <curses.h>

int main() {
	std::cout << "Hello World!";
	// necessary to use curses:
	initscr();
	// allow for tracking input from special keyboard keys (backspace, delete, etc.)
	keypad(stdscr, true);
	
	return 0;
}
