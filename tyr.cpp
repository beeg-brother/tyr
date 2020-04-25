#include <cstdio>
#include <curses.h>
#include <panel.h>
#include <iostream>
#include <fstream>
#include <clocale>
#include <string>
#include <thread>
#include "constants.h"
#include "filemenu.cpp"
#include <filesystem>
#include <vector>
#include <assert.h>
#include <fstream>
#include <zmq.hpp>
#include <map>
#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif


namespace fsys = std::filesystem;
using namespace filemenu;

/* notes on using Cursors:
   line_num should refer to the current line, like normal
   HOWEVER: line_position should refer to the "previous" character
   aka: a Cursor with line_position 0 would add a character at position 0 in the string.
*/
struct Cursor {
	// position on screen
	int screen_x, screen_y;
	// line_num means the nth line in the text (aka the nth element in the string array of the editor)
	// line_position means the position of the cursor in this line (x-value ish)
	int line_num, line_position;
};
// gets the current time as a string timestamp
std::string getCurrentTime(){
	time_t _tm =time(NULL );
	struct tm * curtime = localtime ( &_tm );
	std::string time = asctime(curtime);
	return time;
}

std::map<std::string, std::string> read_config(){
	std::map<std::string, std::string> dict;
	//TODO: error catching for when the .tyrc file doesn't exist
	std::ifstream inputFile(".tyrc");
	std::string line;
	while (std::getline(inputFile, line)){
		// check to see if the line starts with a comment, if so, we ignore it and move on
		if (line[0] == '#' or line[0] == ' '){
			continue;
		}
		else{
			// check to make sure its not doing an empty line
			if (line.size() != 0){	
				// find the occurrence of the first colon:
				int colon_index = line.find(':');
				dict[line.substr(0,colon_index)] = line.substr(colon_index + 1);
			}
		}

	}
	inputFile.close();
	return dict;
}

// logs a message to the location described in .tyrc
void logMessage(std::string message){
	///open the log file
	// get the file path from the config settings
	std::map<std::string, std::string> config_settings = read_config();
	std::string log_file_path = config_settings["log_path"] + "/tyr.log";
	std::ofstream logfile;
	logfile.open(log_file_path, std::ofstream::out | std::ofstream::app);

	// write the current timestamp and the message to it
	std::string toWrite = getCurrentTime() + ": " + message;
	toWrite.erase(std::remove(toWrite.begin(), toWrite.end(), '\n'), toWrite.end());
	logfile << "\n" <<toWrite;
	// close the access to the file
	logfile.close();
}
// clear the log file contents from the last run
void clearLogFile(){
	std::map<std::string, std::string> config_settings = read_config();
	std::string log_file_path = config_settings["log_path"] + "/tyr.log";
	std::ofstream logfile;
	logfile.open(log_file_path, std::ofstream::out | std::ofstream::trunc);
	logfile << "\n";
	logfile.close();
}
// splits a string via the delimiter and returns the substrings as a vector of strings
std::vector<std::string> splitString(std::string message_contents, char delim){
	std::string word = ""; 
	// to count the number of split strings 
	int num = 0; 
	// adding delimiter character at the end 
	// of 'str' 
	message_contents = message_contents + delim; 
  
	// length of 'str' 
	int l = message_contents.size(); 
  
	// traversing 'str' from left to right 
	std::vector<std::string> substr_list; 
	for (int i = 0; i < l; i++) { 
  
		// if str[i] is not equal to the delimiter 
		// character then accumulate it to 'word' 
		if (message_contents[i] != delim) 
			word = word + message_contents[i]; 
  
		else { 
  
			// if 'word' is not an empty string, 
			// then add this 'word' to the array 
			// 'substr_list[]' 
			if ((int)word.size() != 0) 
				substr_list.push_back(word);
			// reset 'word' 
			word = ""; 
		} 
	} 
	// return the splitted strings 
	return substr_list; 
}
// reads the theme file specified in .tyrc and creates a color map, and also initializes the colors
// that are used in the editor.
std::map<std::string,int> theme_setup(std::map<std::string, std::string> config_settings){
	std::string theme_path = config_settings["theme_path"];
	std::string theme_file_name = config_settings["theme"];
	// open up the theme file
	//TODO: check for the existence of the theme file
	std::ifstream inputFile(theme_path + "/" + theme_file_name);
	std::string line;
	std::vector<std::vector<int>> color_vec;
	std::map<std::string,int> color_map;
	// read through the theme file
	while (std::getline(inputFile, line)){
		// ignore commented lines
		if (line[0] != '#'){
			// ignore empty lines
			if (line.size() != 0){
				int colon_index = line.find(':');
				color_map[line.substr(0,colon_index)] = stoi(line.substr(colon_index + 1));
			}
		}
	}
	inputFile.close();

	init_pair(borderFocusedColor, color_map["borderFocused"], -1);
	init_pair(borderUnfocusedColor, color_map["borderUnfocused"], -1);
	init_pair(textColor, color_map["text"], -1);
	init_pair(directoriesColor, color_map["directories"], -1);
	init_pair(filesColor, color_map["files"], -1);
	init_pair(lineNumbersColor, color_map["lineNumbers"], -1);
    init_pair(selectedMenuItemColor,color_map["selectedItem"], -1);
	
	return color_map;
}

int screen_rows, screen_cols;

class Window {
	public:
		int width, height;
		PANEL *pan;
		WINDOW *win;
		PANEL *border_pan;
		WINDOW *border_win;

		virtual void handleInput(int) = 0;
		virtual void onFocus() = 0;
		virtual void deFocus() = 0;
		virtual void resize(int, int) = 0;
		// this method should be called by subclass methods, not called directly
		void create_windows(int outer_h, int outer_w, int outer_y0, int outer_x0, int inner_h, int inner_w, int inner_y0, int inner_x0){
			width = outer_w;
			height = outer_h;
			border_win = newwin(outer_h, outer_w, outer_y0, outer_x0);
			drawBorder(COLOR_PAIR(borderFocusedColor));
			border_pan = new_panel(border_win);
			win = newwin(inner_h, inner_w, inner_y0, inner_x0);
			pan = new_panel(win);
			wrefresh(border_win);
			wrefresh(win);
		}

		void drawBorder(int attrs){
			wborder(border_win, (ACS_VLINE | attrs),
					(ACS_VLINE | attrs),
					(ACS_HLINE | attrs),
					(ACS_HLINE | attrs),
					(ACS_ULCORNER | attrs),
					(ACS_URCORNER | attrs),
					(ACS_LLCORNER | attrs),
					(ACS_LRCORNER | attrs));
		}
};

class Editor : public Window{
	protected:
	public:
		// vector of strings in memory
		std::vector<std::string> strs;

		// the location of the cursor for this window
		Cursor cursor;

		// TODO: implement scrolling
		int scroll_offset;
		// number of columns in the editing window
		int window_width;
		// number of rows in the editing window
		int window_height;


		Editor(int h, int w, int y0, int x0){
			create_windows(h, w, y0, x0);
			cursor = Cursor();
			cursor.screen_x = 0;
			cursor.screen_y = 0;
			strs.push_back(std::string());
			scroll_offset = 0;
			getmaxyx(win, window_height, window_width);
			rewrite();
		}

		// create editor windows, leaving room for line numbering.
		void create_windows(int h, int w, int y0, int x0){
			int line_num_width = 0;
			int last_row_num = scroll_offset + (h - 2);
			// calculate the number of digits in the maximum line number
			while (last_row_num != 0){
				last_row_num /= 10;
				line_num_width++;
			}
			Window::create_windows(h, w, y0, x0, h - 2, w - 2 - line_num_width, y0 + 1, x0 + 1 + line_num_width);
		}

		// returns the WINDOW object that the editing happens in
		WINDOW* getEditorWindow(){
			return Window::win;
		}

		// returns the PANEL object that the editing window is in
		PANEL* getEditorPanel(){
			return Window::pan;
		}

		// TODO: resizing
		void resize(int, int){
			return;
		};

		void rewrite_line_nums(){
			// clear
			wclear(Window::border_win);
			// redraw border
			drawBorder(COLOR_PAIR(borderFocusedColor));
			// count number of digits in max line num
			int num_rows = getmaxy(Window::win);
			int line_num_width = 0;
			while (num_rows != 0){
				num_rows /= 10;
				line_num_width++;
			}
			// use this pointer for the formatting - possible performance gain
			char* out = new char[line_num_width];
			int max = std::min(window_height, static_cast<int>(strs.size()));

			for(int i = scroll_offset + 1; i <= scroll_offset + max; i++){
				// create a right-padded string for the line number
				std::sprintf(out, "%*d", line_num_width, i);
				//TODO: idk where line numbers are written but I don't think this is it
				wattron(win, COLOR_PAIR(lineNumbersColor));
				mvwaddstr(Window::border_win, i, 1, out);
				wattroff(win, COLOR_PAIR(lineNumbersColor));
			}
			// no mem-leaks pls
			delete out;
			wrefresh(border_win);
		}

		// redraws all strings after clearing the screen
		void rewrite(){
			rewrite_line_nums();
			wclear(win);
			// TODO: this doesn't work
			int max = std::min(window_height, static_cast<int>(strs.size()));
			for(int i = 0; i < max; i++){
				wattron(win, COLOR_PAIR(textColor));
				mvwaddnstr(Window::win, i, 0, strs[i].c_str(), window_width);
				wattroff(win, COLOR_PAIR(textColor));
			}
			wrefresh(win);
		}

		void onFocus(){
			return;
		}

		void deFocus(){
			return;
		}

		// deals with the input of characters to the editor.
		// typing, arrow keys, etc.
		void handleInput(int c){
			curs_set(1);
			switch (c){
				case KEY_RIGHT:
					if(strs[cursor.line_num].size() > cursor.line_position){ // check if its valid to move over a character
						cursor.screen_x += 1;
						cursor.line_position += 1;
					} else if(cursor.line_position == strs[cursor.line_num].size()) {
						if(cursor.line_num < strs.size() - 1){
							cursor.line_num += 1;
							cursor.screen_y += 1;
							cursor.line_position = 0;
							cursor.screen_x = 0;
						}
					} else {
						// TODO proper restriction here: case of being at the bottom of the screen with more below, offscreen
						cursor.screen_x = 0;
						cursor.screen_y += 1;
						cursor.line_position = 0;
						cursor.line_num += 1;
					};
						// if not, do nothing
						// TODO: try to shift the line over to continue the view.
					break;
				case KEY_LEFT:
					// TODO: edge cases of line scrolling
					if(cursor.line_position == 0){
						// if at the start of a line, but not line 1, its ok to move to the end of the last line.
						if(cursor.line_num != 0){
							cursor.line_num -= 1;
							cursor.screen_y -= 1;
							cursor.line_position = strs[cursor.line_num].size();
							cursor.screen_x = std::min(window_height, cursor.line_position);
						}
						// if at (0, 0), do nothing
					} else {
						// if we're not at the start of the line, just move left one
						cursor.line_position -= 1;
						cursor.screen_x -= 1;
					}
					break;
				case KEY_UP:
					// TODO: deal with scrolling
					if(cursor.line_num != 0){
						// if at any normal position, just move up a line
						cursor.line_num -= 1;
						// set cursor x to be either the end of the line or the current x, whichever is smaller to prevent out of bounds B)
						cursor.line_position = std::min((int) strs[cursor.line_num].size(), cursor.line_position);
						cursor.screen_y -= 1;
						cursor.screen_x = std::min(window_height, cursor.line_position);
					} else {
						// if at the top line, just move to (0, 0)
						cursor.line_position = 0;
						cursor.screen_x = 0;
					}
					break;
				case KEY_DOWN:
					if(cursor.line_num == strs.size() - 1){
						// if on last line, go to the end
						if(cursor.line_position != strs[cursor.line_num].size()){
							cursor.line_position = strs[cursor.line_num].size();
							cursor.screen_x = std::min((int) strs[cursor.line_num].size(), window_height);
						}
					} else {
						// otherwise, just move down a row
						cursor.line_num += 1;
						cursor.screen_y = std::min(cursor.line_num, screen_rows);
						// again, this line places the cursor at either the current x or the end of the line to avoid out of bounds
						cursor.line_position = std::min(cursor.line_position, (int) strs[cursor.line_num].size());
						cursor.screen_x = std::min(cursor.line_position, window_height);
					}
					break;
				case 10: // ENTER KEY
					if(cursor.line_num == strs.size() - 1){
						// if on last line, we have to use push_back b/c insert doesn't append
						strs.push_back(strs[cursor.line_num].substr(cursor.line_position));
						strs[cursor.line_num] = strs[cursor.line_num].substr(0, cursor.line_position);
					} else {
						// this uses the iterator returned by insert to access the right position
						std::vector<std::string>::iterator temp_it = strs.insert(strs.begin() + cursor.line_num, strs[cursor.line_num].substr(0, cursor.line_position));
						temp_it += 1;
						(*temp_it) = (*temp_it).substr(cursor.line_position);
					}
					// TODO: scrolling
					cursor.screen_y += 1;
					cursor.screen_x = 0;
					cursor.line_num += 1;
					cursor.line_position = 0;

					rewrite();

					break;
				case KEY_BACKSPACE: // BACKSPACE KEY
					if(cursor.line_position == 0){
						// if at the start of a line, we have to combine two lines
						// if at (0, 0), do nothing
						if(cursor.line_num != 0){
							cursor.line_position = strs[cursor.line_num - 1].size();
							cursor.screen_x = std::min(cursor.line_position, window_width);
							strs[cursor.line_num - 1] += strs[cursor.line_num];
							strs.erase(strs.begin() + cursor.line_num);
							cursor.line_num -= 1;
							cursor.screen_y -= 1;
							rewrite();
						}
					} else {
						// if we're not at the start of a line, just remove the previous character
						strs[cursor.line_num].erase(cursor.line_position - 1, 1);
						cursor.screen_x -=1;
						cursor.line_position -= 1;
						// clear the line, then redraw it to update all characters
						mvwaddstr(win, cursor.screen_y, 0, std::string(window_width, ' ').data());
						wattron(win, COLOR_PAIR(textColor));
						mvwaddnstr(win, cursor.screen_y, 0, strs[cursor.line_num].data(), window_width);
                        wattroff(win, COLOR_PAIR(textColor));
					}
					break;
				default:
					// just add the character to the string
					strs[cursor.line_num].insert(cursor.line_position, 1, (char) c);
					const char* a = strs[cursor.line_num].data();
					wattron(win, COLOR_PAIR(textColor));
					mvwaddnstr(Window::win, cursor.screen_y, 0, strs[cursor.line_num].data(), window_width);
					wattroff(win, COLOR_PAIR(textColor));
					wrefresh(Window::win);
					cursor.screen_x += 1;
					cursor.line_position += 1;
					break;
			}
			// update cursor position
			wmove(Window::win, cursor.screen_y, cursor.screen_x);
			wrefresh(Window::border_win);
			wrefresh(Window::win);
		}
};

class FileViewer : public Window{
	protected:
	public:
		FileViewer(int h, int w, int y0, int x0, std::map<std::string,int> color_map){
			create_windows(h, w, y0, x0);
			fsys::path cwd = fsys::current_path();
			nmenu->setWindow(getWindow());
			nmenu->setMenuItems(nmenu->getDirFiles(cwd));
			nmenu->setColorMap(color_map);
			nmenu->drawMenu();

		}
		Cursor cursor;
		// my new version  of menus
		Menu* nmenu = new Menu();

		// TODO: resizing
		void resize(int, int){
			return;
		};

		void create_windows(int h, int w, int y0, int x0){
			Window::create_windows(h, w, y0, x0, h - 2, w - 2, y0 + 1, x0 + 1);
		}

		WINDOW* getWindow(){
			return Window::win;
		}

		void onFocus(){
			return;
		}

		void deFocus(){
			return;
		}

		void handleInput(int c){
			switch(c){
				case KEY_DOWN:
					nmenu->menu_down();
					break;
				case KEY_UP:
					nmenu->menu_up();
					break;
				case KEY_RIGHT:
					nmenu->menu_right();
					break;
				case KEY_LEFT:
					nmenu->menu_left();
					break;
			}
		}
};


class DialogElement {
	public:
		virtual void handleInput(int c) = 0;
		virtual void refresh(WINDOW* win, int i) = 0;
		
};

class ButtonsElement : public DialogElement {
	public:
		void handleInput(int c){
			return;
		}
		void refresh(WINDOW* win, int i){
			return;
		}
};

class StringElement : public DialogElement {
	public:
		const char* message;

		StringElement(const char* str){ // see https://www.oreilly.com/library/view/optimized-c/9781491922057/ch04.html for argument
			message = str;
		}

		void handleInput(int c){
			return;
		}

		void refresh(WINDOW* win, int yval){
			mvwaddstr(win, yval, 0, message);
		}
};

class Dialog : public Window{
	protected:
		std::vector<std::shared_ptr<DialogElement>> elements;
		short currentElement;
	public:

		Dialog(){
			 getmaxyx(stdscr, screen_rows, screen_cols);
			//screen_rows = LINES;
			//screen_cols = COLS;
			// assuming that we want the max width to be 1/3 of the screen (idk i just picked this #)
			int width = (screen_cols + 2)/3; // this looks jank but stack overflow says this will round the division up
			
			// TODO: determine the number of rows
			int height = 5;

			// center dialog box
			int start_x = (screen_cols + 1)/2 - (width + 1)/2; // again, rounding up the division
			int start_y = (screen_rows + 1)/2 - (height + 1)/2;
			// offsets to account for the border
			Window::create_windows(height + 2, width + 2, start_y - 1, start_x - 1, height, width, start_y, start_x);
			elements.push_back(std::make_shared<StringElement>("testing"));
			refresh();
			wrefresh(Window::win);
		}

		void refresh(){
			for(int i = 0; i < elements.size(); i++){
				wattron(win, COLOR_PAIR(textColor));
				elements[i]->refresh(Window::win, i);
				wattroff(win, COLOR_PAIR(textColor));
			}
			wrefresh(Window::win);
		}

		void handleInput(int c){
			switch(c){
				case(KEY_STAB) : {
					currentElement = (currentElement + 1) % elements.size();
				}
				case(KEY_BTAB) : {
					currentElement = (currentElement - 1 + elements.size()) % elements.size();
				}
			};
		}

	// TODO: these
	void resize(int, int){
		return;
	}

	void onFocus(){
		return;
	}

	void deFocus(){
		return;
	}
};

Editor *ed;
FileViewer *fs;
Window *focused;

// TODO: make mainLoop deal with sending input to the right window.... :(
void mainLoop(){
	int c;
	while(1){
		c = getch();
//		switch(c){
//		    case (KEY_ATAB & 037):
//		        focused->deFocus();
//
//		}
		focused->handleInput(c);
	};
}

// takes in a command string from a plugin and returns the message that tyr will send back
// it also changes anything that needs to be changed according to the plugin's message
//TODO: get evan to write the parts involving the editor
//TODO: write the parts that don't involve the editor
std::string parseMessage(std::string message_contents){
	std::string reply;
	// turn the string into an array of strings, each representing the items
	std::vector<std::string> message_items = splitString(message_contents,',');
	// if the message is a getter request
	if (message_items[0] == "g"){
		// if the plugin is asking for the editor cursor position
		if (message_items[1] == "ed_curs"){
			// evan idk how to do this so you're gonna need to do this
		}
		// if the plugin is asking for the currently selected item in the file viewer
		if (message_items[1] == "file_curs"){
			reply = fs->nmenu->getCurrentItem().u8string();
		}
		// if the plugin is asking for the character at a certain location
		if (message_items[1] == "ed_char"){
			// look at the arguments provided (i guess they'll be coords)
			int x_coord = stoi(message_items[2]);
			int y_coord = stoi(message_items[3]);
			// some logic that gets the character at that location in the editor window
		}
		// if the plugin is asking for the filename of the currently open file:
		if (message_items[1] == "filename"){
			// file opening is not currently implemented yet so we cry
		}
		// if the plugin is asking for the length of the currently open file
		if (message_items[1] == "num_lines"){
			// cry
		}
		if (message_items[1] == ""){

		}
	}



	// if the message is a setter
	if (message_items[0] == "s"){
		// these next 4 are just file viewer movement requests
		if (message_items[1] == "file_down"){
			fs->nmenu->menu_down();
			reply = "file viewer moved down";
		}
		if (message_items[1] == "file_up"){
			fs->nmenu->menu_up();
			reply = "file viewer moved up";
		}
		if (message_items[1] == "file_collapse"){
			fs->nmenu->menu_left();
			reply = "file viewer moved left";
		}
		if (message_items[1] == "file_expand"){
			fs->nmenu->menu_right();
			reply = "file viewer moved right";
		}

	}
	return reply;
}



void start_server(std::string ipc_path){
	// create the zmq context
	zmq::context_t context (1);
	logMessage("Created zmq context.");
	// create the socket that we will be binding
	zmq::socket_t tyr_socket (context, ZMQ_REP);
	logMessage("Created zmq socket");
	// bind it to the ipc port/path that plugins will be communicating on
	tyr_socket.bind(ipc_path);
	logMessage("Bound socket to ipc path");
	// begin the loop of talking to plugins
	while (true){
		// wait for the plugin to send info
		zmq::message_t message;
		tyr_socket.recv(&message);
		std::string message_contents = std::string(static_cast<char*>(message.data()), message.size()); 
		logMessage("Received message from plugin");
		// parse the message data and do stuff here
		std::string response;
		response = parseMessage(message_contents);
		//send the message to the plugin with what the plugin requested or w/e
		zmq::message_t reply (response.length());
		memcpy(reply.data(), (const void*) response.c_str(), response.length());
		tyr_socket.send(reply);
		logMessage("Sent response to plugin");
	}
}

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
	getmaxyx(stdscr, screen_rows, screen_cols);
}


int main() {
	curses_setup();
	clearLogFile();
	// read the config file
	std::map<std::string,std::string> config_settings = read_config();
	std::map<std::string,int> color_map = theme_setup(config_settings);
	// set up the ipc path according to the config file
	std::string ipc_path = config_settings["ipc_path"];
	logMessage("Obtained ipc plugin path");
	
	// creates the editor screen
	ed = new Editor(screen_rows, screen_cols-20, 0, 20);
	fs = new FileViewer(screen_rows, 21, 0, 0, color_map);
	//Dialog * dia = new Dialog();
	focused = fs;
	logMessage("Created editor and fileviewer objects");
	update_panels();
	doupdate();

	// creating a thread to house the plugin server
	std::thread server_thread (start_server, ipc_path);
	logMessage("Created server thread for ipc server");
	curs_set(1);
	mainLoop();
	// close curses
	logMessage("Closing tyr");
	endwin();
	return 0;
}
