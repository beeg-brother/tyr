#include "window.h"
#include "dialog.h"
#include "filebrowser.h"
#include "editor.h"
#include "cursor.h"
#include "constants.h"
#include <cstdio>
#include <curses.h>
#include <panel.h>
#include <iostream>
#include <fstream>
#include <clocale>
#include <string>
#include <thread>
#include <filesystem>
#include <vector>
#include <assert.h>
#include <fstream>
#include <zmq.hpp>
#include <map>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif


namespace fsys = std::filesystem;


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
	// open the log file
	// get the file path from the config settings
	std::map<std::string, std::string> config_settings = read_config();
	std::string log_file_path = config_settings["log_path"] + "/tyr.log";
	std::ofstream logfile;
	logfile.open(log_file_path, std::ofstream::out | std::ofstream::app);

	// write the current timestamp and the message to it
	std::string toWrite = getCurrentTime() + ": " + message;
	toWrite.erase(std::remove(toWrite.begin(), toWrite.end(), '\n'), toWrite.end());
	logfile << toWrite << "\n";
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
				logMessage(line.substr(0, colon_index) + " = " + std::to_string(color_map.at(line.substr(0, colon_index))));
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


editor::Editor *ed;
filebrowser::FileBrowser *fb;
window::Window *focused;

// TODO: window switching
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
		reply = fb->getCurrentItem().u8string();
	}
	if (message_items[0] == "set_file_curs"){
		// get the index of the file that we're looking for
		int index = fb->getIndexOf(message_items[1]);
		// check to make sure that the thing we are looking for exists
		if (index != -1){
			fb->setCurrentItem(index);
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
		fb->menu_down();
		reply = "file viewer moved down";
	}
	if (message_items[0] == "file_up"){
		fb->menu_up();
		reply = "file viewer moved up";
	}
	if (message_items[0] == "file_collapse"){
		fb->menu_left();
		reply = "file viewer collapsed current item";
	}
	if (message_items[0] == "file_expand"){
		fb->menu_right();
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
	ed = new editor::Editor(screen_rows, screen_cols-20, 0, 20, std::filesystem::path("example"));
	fb = new filebrowser::FileBrowser(screen_rows, 21, 0, 0, color_map);
    fb->drawMenu();
	std::vector<std::shared_ptr<dialog::DialogElement>> el;

    std::vector<std::string> test {"tetsing", "test"};
    std::vector<std::string> test2 {"tetsing2", "test2"};
    el.push_back(std::make_shared<dialog::ButtonsElement>(test, false));
    el.push_back(std::make_shared<dialog::InputElement>(1));
    el.push_back(std::make_shared<dialog::StringElement>("string test", 11));

	dialog::Dialog * dia = new dialog::Dialog(el);
	focused = ed;
	logMessage("Created editor and filebrowser objects");
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
