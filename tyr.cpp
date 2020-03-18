
#include <curses.h>
#include <panel.h>
#include <iostream>
#include <clocale>
#include <string>
#include "filemenu.cpp"
#include <menu.h>
#include <filesystem>
#include <vector>
#include <assert.h>
#include <fstream>



namespace fsys = std::filesystem;
using namespace filemenu;

struct Cursor {
	int x, y;
};


class Window {
	protected:
		int width, height;
		PANEL *pan;
		WINDOW *win;
		PANEL *border_pan;
		WINDOW *border_win;
	public:
		void resize(int, int);
		void onFocus();
		void create_windows(int h, int w, int y0, int x0){
			width = w;
			height = h;
			win = newwin(h - 2, w - 2, y0 + 1, x0 + 1);
			pan = new_panel(win);
			border_win = newwin(h, w, y0, x0);
			wborder(border_win, 0, 0, 0, 0, 0, 0, 0, 0);
			border_pan = new_panel(border_win);
			wrefresh(border_win);
			wrefresh(win);
		}

};

class Editor : protected Window{
	protected:
	public:
		Editor(int h, int w, int y0, int x0){
			Window::create_windows(h, w, y0, x0);
		}
		Cursor cursor;
		void resize(int, int);

		void onFocus();
};

std::vector<std::string> getDirFiles(std::string path){
	// create the list of files
	std::vector<std::string> files;
	// get the other files in the directory
	for (const auto & entry : fsys::directory_iterator(path)){
		files.push_back(entry.path().filename());
	}
	return files;
}

class FileViewer : protected Window{
	protected:
	public:

		FileViewer(int h, int w, int y0, int x0){
			Window::create_windows(h, w, y0, x0);
		}
		Cursor cursor;
		// my new version  of menus
		Menu* nmenu = new Menu();
		void resize(int, int);

		WINDOW* getWindow(){
			return Window::win;
		}

};

void menuInputHandling(FileViewer fs){
	int c;
	while((c = getch()) != KEY_BACKSPACE){
		switch(c){
			case KEY_DOWN:
				fs.nmenu->menu_down();
				break;
			case KEY_UP:
				fs.nmenu->menu_up();
				break;
			case KEY_RIGHT:
				fs.nmenu->menu_right();
				break;
			case KEY_LEFT:
				fs.nmenu->menu_left();
				break;
		}
	}
}

int main() {
	// sets the locale so that terminals use UTF8 encoding
	//std::setlocale(LC_ALL, "en_US.UTF-8");
	// initializes curses
	initscr();
	//starts color system
	start_color();
	// refreshes the screen
	keypad(stdscr, TRUE);
	refresh();
	cbreak();
	noecho();
	
	// creates the editor screen
	Editor ed (LINES-1, COLS-20, 0, 20);
	FileViewer fs (LINES-1, 21, 0, 0);
	//update the panel stacking
	update_panels();
	doupdate();
	
	fsys::path cwd = fsys::current_path();
	fs.nmenu->setWindow(fs.getWindow());
	fs.nmenu->setMenuItems(fs.nmenu->getDirFiles(cwd));
	fs.nmenu->drawMenu();
	menuInputHandling(fs);
	//focusOnFileViewer(fs);

	getch();
	// close curses
	endwin();
	return 0;
}

