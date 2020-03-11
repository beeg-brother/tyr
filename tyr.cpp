
#include <curses.h>
#include <panel.h>
#include <iostream>
#include <clocale>
#include <string>
#include "filemenu.h"
#include <menu.h>
#include <filesystem>
#include <vector>
#include <assert.h>
#include <fstream>



namespace fsys = std::filesystem;

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
		MENU * menu;
		void resize(int, int);

		WINDOW* getWindow(){
			return Window::win;
		}

		void setMenu(ITEM ** menu_choices){
			// create a new menu
			menu = new_menu(menu_choices);
			set_menu_win(menu, Window::win);
			post_menu(menu);
			wrefresh(Window::win);
		}
		MENU* getMenu(){
			return menu;
		}

		void expandDirectory(ITEM** menu_choices, std::string directory){
			std::vector<std::string> new_files = getDirFiles(directory);
			// get the number of new files we're adding
			int num_new_files = new_files.size();
			// make the new menu choices array the right size
			ITEM* new_menu_choices[num_new_files + item_count(menu) + 1] = {};
			// get the index of the directory we expanded
			ITEM* item = current_item(menu);
			int expandedIndex = item_index(item);
			
			// for everything before the expanded index we're good
			for (int i = 0; i < expandedIndex; i++){
				new_menu_choices[i] = menu_choices[i];
			}
			new_menu_choices[expandedIndex] = new_item(directory.c_str(),"v");
			
			// add the subfiles
			for (int i = 1 ; i <= num_new_files; i++){
				new_menu_choices[expandedIndex + i] = new_item(new_files[i - 1].c_str(),"s");
			}
			// add the files that come after the subfiles
			for (int i = expandedIndex + num_new_files + 1; i <= num_new_files + item_count(menu); i++){
				new_menu_choices[i] = menu_choices[i - num_new_files];
			}
			new_menu_choices[item_count(menu) + num_new_files + 1] = (ITEM *) NULL;
			
			unpost_menu(menu);
			set_menu_items(menu, new_menu_choices);
			post_menu(menu);
			set_current_item(menu, new_menu_choices[expandedIndex]);
			wrefresh(Window::win);
		}
		void resizeMenu(int rows, int cols){
			menu_format(menu, &rows, &cols);
			wrefresh(Window::win);
		}
};



void focusOnFileViewer(FileViewer fs){
	// the base path (will have to be based off of pwd)
	fsys::path cwd = fsys::current_path(); 
	std::string path = cwd.u8string();
	// get a vector of the files in the path
	std::vector<std::string> files = getDirFiles(path);
	// get the number of total files
	int num_choices = files.size();
	// make an empty vector of menu items
	ITEM* menu_choices[num_choices + 1] = {};
	// populate the menu array
	for (int i = 0; i < num_choices-1; i++){
		const fsys::path fspath(files[i]);
		const char *cstr = files[i].c_str();

		// make a new menu item that's the name of the file
		if (fsys::is_directory(fspath)){
			menu_choices[i] = new_item(cstr,">");    
		}
		if (fsys::is_regular_file(fspath)){
			menu_choices[i] = new_item(cstr,"");   
		}
	}
	menu_choices[num_choices] = (ITEM *) NULL;
	fs.setMenu(menu_choices);
	ITEM **items;
	// when we switch to the file viewer we want to hide the cursor
	curs_set(0);
	// refresh the file viewer window
	wrefresh(fs.getWindow());
	int c;
	while((c = getch()) != KEY_BACKSPACE)
	{   switch(c)
		{   case KEY_DOWN:
				menu_driver(fs.getMenu(), REQ_DOWN_ITEM);
				wrefresh(fs.getWindow());
				break;
			case KEY_UP:
				menu_driver(fs.getMenu(), REQ_UP_ITEM);
				wrefresh(fs.getWindow());
				break;
			// when the user selects a file from the menu
			case KEY_RIGHT:
				items = menu_items(fs.getMenu());

				const char* name = item_name(current_item(fs.getMenu()));
				std::string file_name (name);
				const fsys::path clicked_path (file_name);

				if (fsys::is_directory(clicked_path)){
					
					//expand the files inside the sub-directory
					fs.expandDirectory(items, file_name);
					items = menu_items(fs.getMenu());
					wrefresh(fs.getWindow());
				}
				if (fsys::is_regular_file(clicked_path)){
					// open the file in the editor

					wrefresh(fs.getWindow());
				}
		}
	}

}

int main() {
	// sets the locale so that terminals use UTF8 encoding
	//std::setlocale(LC_ALL, "en_US.UTF-8");
	// initializes curses
	initscr();
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

	focusOnFileViewer(fs);

	getch();
	// close curses
	endwin();
	return 0;
}

