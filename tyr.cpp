#include "window.h"
#include "editor.h"
#include "cursor.h"
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

// draw a border using line graphics to the rectangle described by the points (x1, y1) and (x2, y2)
// (x1, y1) is higher and more to the left than (x2, y2), swapping this order breaks it
// note that as is customary with curses, y-values come first in the argument list
// win: the WINDOW to draw to
// attrs: any attributes to combine with the line characters (namely colors)
// y1: y-value of the upper left point
// x1: x-value of the upper left point
// y2: y-value of the lower right point
// x2: x-value of the lower right point
void draw_box(WINDOW* win, int attrs, int y1, int x1, int y2, int x2){
    int h = y2 - y1;
    int w = x2 - x1;
    wmove(win, y1, x1);
    wvline(win, ACS_VLINE | attrs, h);
    waddch(win, ACS_ULCORNER | attrs);
    whline(win, ACS_HLINE | attrs, w - 1);
    wmove(win, y1, x2);
    wvline(win, ACS_VLINE | attrs, h);
    waddch(win, ACS_URCORNER | attrs);
    wmove(win, y2, x1);
    whline(win, ACS_HLINE | attrs, w);
    waddch(win, ACS_LLCORNER | attrs);
    wmove(win, y2, x2);
    waddch(win, ACS_LRCORNER | attrs);
}

int screen_rows, screen_cols;

class FileViewer : public window::Window{
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

// TODO: make only some kinds of DialogElements focusable
class DialogElement {
    public:
        // is the user focused on this element?
        bool isFocused = false;

        // handles input characters
        // some characters are intercepted by the Dialog class:
        //     * tab / shift+tab
        //     * up / down
        // that's all so far
        // c: the character needing to be handled
        virtual void handleInput(int c) = 0;

        // refreshes the DialogElement on the window
        // win: the window the Dialog is painted on
        // startY: the first y-value the DialogElement is allowed to use
        //         elements can use from startY to startY+requestNumLines(width)
        //         painting outside of this area will be overwritten if there is another element below
        //         elements can use the entire width of the window
        virtual void refresh(WINDOW* win, int startY) = 0;

        // any actions that need to be done when the user focuses on the element
        // for example: changing cursor settings (visible/invis), changing border colors to look focused, etc
        virtual void onFocus() = 0;

        // any actions that need to be done as the user switches focus away from the element
        // for example: resetting cursor settings, changing border colors to look unfocused, etc
        virtual void deFocus() = 0;

        // this allows the DialogElement to reserve space in the window based on a width provided
        // note that this does not guarantee that the next refresh() will use a window with this width
        // storing width in the DialogElement is highly discouraged as it is generally pointless
        // width: the size of the hypothetical window
        virtual int requestNumLines(int width) = 0;

        // this is called on the focused element of a Dialog window on each refresh
        // it allows the DialogElement to place the cursor where it needs to be for the Dialog appearance
        // this basically only applies for the InputElement, but it might be useful in the future
        // note that it is perfectly valid behavior to just return if the cursor is invisible anyway - efficiency
        // win: the window the Dialog is painted on
        // startY: the first y-value that the DialogElement was allowed to draw in (see refresh() )
        virtual void placeCursor(WINDOW* win, int startY) = 0;
};

class InputElement : public DialogElement{
    public:
        std::string contents;
        int numLines;
        int position;
        // TODO: implement horizontal scrolling

        InputElement(int lines){
            // TODO: implement lines
            numLines = lines;
            position = 0;
        }

        void onFocus(){
            isFocused = true;
            curs_set(1);
            return;
        }

        void deFocus(){
            isFocused = false;
            curs_set(1);
            return;
        }

        int requestNumLines(int width){
            // TODO: implement lines
            return 3;
        }

        void refresh(WINDOW* win, int startY){
            draw_box(win, COLOR_PAIR(borderFocusedColor), startY, 0, startY + 2, getmaxx(win) - 1);
//            attron(COLOR_PAIR(textColor));
            mvwaddnstr(win, startY + 1, 1, contents.data(), getmaxx(win) - 2);
//            attroff(COLOR_PAIR(textColor));
            wmove(win, startY + 1, position + 1);
        }

        void placeCursor(WINDOW* win, int startY){
            wmove(win, startY+1, std::min(getmaxx(win) - 2, position + 1));
        }

        void handleInput(int c){
            // TODO: move cursor to proper position
            logMessage("InputElement received " + std::to_string(KEY_BACKSPACE));
            switch(c){
                case KEY_RIGHT :
                    // TODO: bounding based on window, moving cursor
                    position++;
                    break;
                case KEY_LEFT :
                    position--;
                    break;
                case 127 :
                    // TODO: clearing old chars
                    if (position != 0){
                        contents.erase(position - 1, 1);
                        position--;
                    }
                    break;
                default:
                    contents.insert(position, 1, (char) c);
                    position++;

            }
        }
};

// used to provide button options to the user
// selecting a button can close the dialog if the exitOnSelect flag is true
// otherwise, selected button(s) have a distinct highlight
class ButtonsElement : public DialogElement {
    public:
        std::vector<std::string> options;
        std::vector<std::string>::iterator selected;
        short numOptions;
        bool eos;
        // TODO: this
        short intendedYLevel = 3;

        ButtonsElement(std::vector<std::string> opts, bool exitOnSelect){
            options = opts;
            numOptions = opts.size();
            eos = exitOnSelect;
            selected = options.begin();
        }

        void onFocus(){
            isFocused = true;
            curs_set(0);
            return;
        }

        void deFocus(){
            isFocused = false;
            curs_set(1);
            return;
        }

        int requestNumLines(int width){
            // TODO: make this accurate
            return 3;
        }

        void refresh(WINDOW* win, int startY){
            // TODO: center buttons
            intendedYLevel = startY + 1;
            int currentX = 0;
            for(std::string s: options){
                if (isFocused & (s == *selected)){
                    draw_box(win, COLOR_PAIR(borderFocusedColor), intendedYLevel - 1, currentX, intendedYLevel + 1, currentX + s.size() + 1);
                } else {
                    draw_box(win, COLOR_PAIR(borderUnfocusedColor), intendedYLevel - 1, currentX, intendedYLevel + 1, currentX + s.size() + 1);
                }
                mvwaddstr(win, intendedYLevel, currentX + 1, s.data());
                currentX += s.size() + 2;
            }
            return;
        }

        void placeCursor(WINDOW* win, int startY){
            return;
        }

        void handleInput(int c){
            logMessage("ButtonDialog received input: " + std::to_string(c));
            switch(c){
                case KEY_RIGHT:
                    logMessage(*selected);
                    selected++;
                    if (selected == options.end()){
                        selected = options.begin();
                    }
                    break;
                case KEY_LEFT:
                    logMessage("left");
                    if (selected == options.begin()){
                        selected = options.end() - 1;
                    } else {
                        selected--;
                    }
                    break;
                case KEY_ENTER:
                    if(eos){
                        // TODO: returning values to where they should go
                        return;
                    }
                    else {
                        return;
                    }
                    break;
                default:
                    break;
            }
            return;
        }

};

// used to display a string to the user
class StringElement : public DialogElement {
	public:
		const char* message;
		int length;

        StringElement(const char* str, int len){ // see https://www.oreilly.com/library/view/optimized-c/9781491922057/ch04.html for argument
            message = str;
            length = len;
        }
        ~StringElement(){
            delete message;
        }

        void onFocus(){
            isFocused = true;
            curs_set(0);
            return;
        }

        void deFocus(){
            isFocused = false;
            curs_set(1);
            return;
        }

        int requestNumLines(int width){
            // round up division. this prolly sucks tbh
            return length / width + (length % width != 0);
        }

        void refresh(WINDOW* win, int yval){
            mvwaddstr(win, yval, 0, message);
        }

        void placeCursor(WINDOW* win, int startY){
            return;
        }

        void handleInput(int c){
            // do nothing
            return;
        }

};

class Dialog : public window::Window{
	protected:
		std::vector<std::shared_ptr<DialogElement>> elements;
        std::vector<std::shared_ptr<DialogElement>>::iterator currentElement;
        // note that this is not guaranteed to be accurate, but it was updated the last time the Dialog was resized
        // if it is empty on a refresh, it is initialized
        std::vector<int> cached_startYs_cumulative;

	public:

        Dialog(std::vector<std::shared_ptr<DialogElement>> els){
             getmaxyx(stdscr, screen_rows, screen_cols);

            // assuming that we want the max width to be 1/3 of the screen (idk i just picked this #)
            int width = (screen_cols + 2)/3; // this looks jank but stack overflow says this will round the division up

            elements = els;

            // this is hacky, unexpected, and does nothing right now
            elements.push_back(std::make_shared<ButtonsElement>(std::vector<std::string> {"exit"}, false));

            int lines = 0;
            for(std::vector<std::shared_ptr<DialogElement>>::iterator it = elements.begin(); it != elements.end(); it++){
                lines += (*it)->requestNumLines(width);
            }
            height = lines;

            // center dialog box
            int start_x = (screen_cols + 1)/2 - (width + 1)/2; // again, rounding up the division
            int start_y = (screen_rows + 1)/2 - (height + 1)/2;

            // offsets to account for the border
            Window::create_windows(height + 2, width + 2, start_y - 1, start_x - 1, height, width, start_y, start_x);
            currentElement = elements.begin();

            (*currentElement)->onFocus();

            refresh();
            wrefresh(Window::win);
        }

        void reloadHeights(){
            int currY = 0;
            cached_startYs_cumulative.clear();
            for(int i = 0; i < elements.size(); i++){
                cached_startYs_cumulative.push_back(currY);
                currY += elements[i]->requestNumLines(width);
            }
        }

		void refresh(){
            if(cached_startYs_cumulative.empty()){
                reloadHeights();
            }
			for(int i = 0; i < elements.size(); i++){
				wattron(win, COLOR_PAIR(textColor));
				elements[i]->refresh(Window::win, cached_startYs_cumulative[i]);
				wattroff(win, COLOR_PAIR(textColor));
			}
			// note: it - vector.begin() gets the index of iterator it
            (*currentElement)->placeCursor(win, cached_startYs_cumulative[currentElement - elements.begin()]);
			wrefresh(Window::win);
		}

		void handleInput(int c){
			switch(c){
				case('\t') :
				case(KEY_DOWN) :
                    (*currentElement)->deFocus();
					currentElement++;
					if (currentElement == elements.end()){
					    currentElement = elements.begin();
					}
                    (*currentElement)->onFocus();
				    break;

				case(KEY_BTAB) :
				case(KEY_UP) :
                    (*currentElement)->deFocus();
                    if (currentElement == elements.begin()){
                        currentElement = elements.end();
                    }
                    currentElement--;
                    (*currentElement)->onFocus();
				    break;

				default:
                    (*currentElement)->handleInput(c);
				    break;
			};
			refresh();
		}

	// TODO: these
	void resize(int, int){
		return;
	}

	void onFocus(){
        (*currentElement)->onFocus();
        return;
	}

	void deFocus(){
        (*currentElement)->deFocus();
		return;
	}
};

editor::Editor *ed;
FileViewer *fs;
window::Window *focused;

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

	// if the plugin is asking for the editor cursor position
	if (message_items[0] == "get_ed_curs"){
		// evan idk how to do this so you're gonna need to do this
	}
	if(message_items[0] == "set_ed_curs"){
		// also don't know how to do this
	}
	// if the plugin is asking for the currently selected item in the file viewer
	if (message_items[0] == "get_file_curs"){
		reply = fs->nmenu->getCurrentItem().u8string();
	}
	if (message_items[0] == "set_file_curs"){
		// get the index of the file that we're looking for
		int index = fs->nmenu->getIndexOf(message_items[1]);
		// check to make sure that the thing we are looking for exists
		if (index != -1){
			fs->nmenu->setCurrentItem(index);
			reply = "set file cursor to " + message_items[1];
		}
		else{
			reply = "that path is not in the file menu";
		}

	}
	// if the plugin is asking for the character at a certain location
	if (message_items[0] == "get_ed_char"){
		// look at the arguments provided (i guess they'll be coords)
		int x_coord = stoi(message_items[1]);
		int y_coord = stoi(message_items[2]);
		// some logic that gets the character at that location in the editor window
	}
	if (message_items[0] == "set_ed_char"){
		// look at the arguments provided (i guess they'll be coords)
		int x_coord = stoi(message_items[1]);
		int y_coord = stoi(message_items[2]);
		// get the character that should be placed there
		char new_char = message_items[3].c_str()[0];
		// some logic that sets the character at the coordinates

	}
	// if the plugin is asking for the filename of the currently open file:
	if (message_items[0] == "get_filename"){
		// file opening is not currently implemented yet so we cry
	}
	// if the plugin is asking for the length of the currently open file
	if (message_items[0] == "get_num_lines"){
		// cry
	}
	// these next 4 are just file viewer movement requests
	if (message_items[0] == "file_down"){
		fs->nmenu->menu_down();
		reply = "file viewer moved down";
	}
	if (message_items[0] == "file_up"){
		fs->nmenu->menu_up();
		reply = "file viewer moved up";
	}
	if (message_items[0] == "file_collapse"){
		fs->nmenu->menu_left();
		reply = "file viewer collapsed current item";
	}
	if (message_items[0] == "file_expand"){
		fs->nmenu->menu_right();
		reply = "file viewer expanded current item";
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
	ed = new editor::Editor(screen_rows, screen_cols-20, 0, 20);
	fs = new FileViewer(screen_rows, 21, 0, 0, color_map);

	std::vector<std::shared_ptr<DialogElement>> el;

    std::vector<std::string> test {"tetsing", "test"};
    std::vector<std::string> test2 {"tetsing2", "test2"};
    el.push_back(std::make_shared<ButtonsElement>(test, false));
    el.push_back(std::make_shared<InputElement>(1));
    el.push_back(std::make_shared<StringElement>("string test", 11));



	Dialog * dia = new Dialog(el);
	focused = dia;
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
