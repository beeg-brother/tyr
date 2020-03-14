#include <curses.h>
#include <panel.h>
#include <iostream>
#include <clocale>
#include <string>
#include <menu.h>
#include <filesystem>
#include <vector>
#include <assert.h>
#include <fstream>

namespace fsys = std::filesystem;
// https://solarianprogrammer.com/2019/01/13/cpp-17-filesystem-write-file-watcher-monitor/
namespace filemenu{
	class Menu {
		protected:
		public:
			// the list of choices that will show up in the menu
			std::vector<fsys::path> menu_choices;
			// determines whether or not an item in the menu is expanded or not
			std::vector<bool> expansion_vector;
			// the number of files in the menu choices
			int num_files;
			// the root path, where tyr is opened from
			fsys::path cwd = fsys::current_path();
			// the window that the menu will display in
			WINDOW* win;
			// drawing related things:
			int window_width;
			int window_height;
			int scroll_start = 0;
			//selected item color
			
			//init_pair(0, COLOR_GREEN,COLOR_BLACK);
			
			// deselected item color
			
			//init_pair(1, COLOR_WHITE,COLOR_BLACK);

			// the index of the currently selected item
			int current_index;
			Menu(){

			}
			Menu(WINDOW* window){
				win = window;
			}
			Menu(std::vector<fsys::path> choices){
				menu_choices = choices;
				num_files = menu_choices.size();
				expansion_vector.resize(num_files,false);
			}
			Menu(std::vector<fsys::path> choices, WINDOW* window){
				win = window;
				menu_choices = choices;
				num_files = menu_choices.size();
				expansion_vector.resize(num_files,false);
			}


			// sets the window in which the menu will appear
			void setWindow(WINDOW* window){
				win = window;
				getmaxyx(win, window_height, window_width);
			}
			// draws the menu onto the screen
			void drawMenu(){
				num_files = menu_choices.size();
				// check to make sure that there is a window that the menu is attached to
				if (win != nullptr){
					curs_set(0);
					for (int currenty = scroll_start; currenty < scroll_start + window_height; currenty++){
						if (currenty < num_files){
							if (currenty == current_index){
								wattron(win,A_BOLD | A_UNDERLINE);
								waddnstr(win, menu_choices[currenty].filename().u8string().c_str(),window_width - 1);
								wattroff(win,A_BOLD| A_UNDERLINE);
								waddstr(win, "\n");
							}
							
							else{
								if (fsys::is_directory(menu_choices[currenty])){
									wattron(win,A_UNDERLINE);
									waddnstr(win, menu_choices[currenty].filename().u8string().c_str(),window_width - 1);
									wattroff(win, A_UNDERLINE);
									waddstr(win, "\n");
								}
								else{
									// items that aren't selected
									waddnstr(win, menu_choices[currenty].filename().u8string().c_str(),window_width - 1);
									waddstr(win, "\n");
								}

							}

						}
					}
				}
				wrefresh(win);
			}
			//removes the menu from the screen
			void removeMenu(){
				// check to make sure that there is a window that the menu is attached to
				if (win != nullptr){
					werase(win);
				}
			}
			// gets files from the given directory as a vector of strings
			std::vector<fsys::path> getDirFiles(fsys::path path){
				// create the list of files
				std::vector<fsys::path> files;
				// get the other files in the directory
				for (const auto & entry : fsys::directory_iterator(path)){
					files.push_back(entry.path());
				}
				return files;
			}

			// updates the menu on the screen
			// takes in the new elements and the position in which they are supposed to be put in
			void updateMenu(std::vector<fsys::path> new_elements, int index){
				// insert the elements into the menu_choices in the correct index
				menu_choices.insert(menu_choices.begin() + index + 1,new_elements.begin(), new_elements.end());
				num_files = menu_choices.size();
				expansion_vector.resize(num_files, false);
				expansion_vector[index] = true;
			}
			void setMenuItems(std::vector<fsys::path> path_vector){
				menu_choices = path_vector;
				num_files = path_vector.size();
				expansion_vector.resize(num_files, false);
			}
			void collapseDirectory(int directoryIndex){
				// figure out how many items are in the directory
				// take out that many items from right after directoryIndex
				// make the directory expandable again
			}

			void expandDirectory(int directoryIndex){
				fsys::path path_expanding = menu_choices[directoryIndex];
				std::vector<fsys::path> new_elements;
				new_elements = getDirFiles(path_expanding);
				removeMenu();
				wrefresh(win);
				updateMenu(new_elements, directoryIndex);
				drawMenu();
			}

			void checkForFileUpdates(fsys::path path){

			}

			void setCurrentItem(int index){
				current_index = index;
			}

			void setCurrentItem(fsys::path path){

			}

			std::vector<fsys::path> getMenuItems(){
				return menu_choices;
			}

			fsys::path getCurrentItem(){
				return menu_choices[current_index];
			}

			void menu_down(){
				if (current_index == num_files - 1){
					current_index = 0;
				}

				else{
					current_index += 1;
				}
				removeMenu();
				scrollToFit();
				drawMenu();
			}
			void menu_up(){
				if (current_index == 0){
					current_index = num_files -1;
				}
				else {
					current_index -= 1;
				}
				removeMenu();
				scrollToFit();
				drawMenu();
			}
			void menu_left(){
				// if its a directory, collapse it
				if (fsys::is_directory(menu_choices[current_index])){
					collapseDirectory(current_index);
				}
				wrefresh(win);
			}
			void menu_right(){
				// if its a directory expand it
				if (fsys::is_directory(menu_choices[current_index])){
					if (expansion_vector[current_index] == false){
						expandDirectory(current_index);
					}
				}
				wrefresh(win);
			}

			void scrollToFit(){
				// if the current index is outside of the range
				// we need to scroll to fit the current selection onto the screen
				while (current_index < scroll_start){
					scroll_start -= 1;
				}
				while (current_index > scroll_start + window_height - 1){
					scroll_start += 1;
				}

			}
	};
}