#ifndef FILEMENU_H
#define FILEMENU_H

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

namespace filemenu {
	class Menu{
		public:
			//constructors
			Menu();
			Menu(WINDOW* window);
			Menu(std::vector<fsys::path> choices);
			Menu(std::vector<fsys::path> choices, WINDOW* window);
			// sets the window that the menu will be drawn onto
			void setWindow(WINDOW* window);
			// draws the menu onto the window
			void drawMenu();
			// remove the menu from the window
			void removeMenu();
			// update the menu by adding the new elements that the index given
			void updateMenu(std::vector<fsys::path> new_elements, int index);
			// collapse a directory
			void collapseDirectory(int directoryIndex);
			// expands a directory to show its subdirectories
			void expandDirectory(int directoryIndex);
			// check the path for file changes and update the menu
			void checkForFileUpdates(fsys::path path);
			// sets the selection to the item at the index
			void setCurrentItem(int index);
			// sets the current selection to the item with the same path
			void setCurrentItem(fsys::path path);
			// returrns the current list of menu items
			std::vector<fsys::path> getMenuItems();
			// returns the current item that the user is selecting
			fsys::path getCurrentItem();
			//input handling for the menu
			void menu_down();
			void menu_up();
			void menu_right();
			void menu_left();

			
	};
}

#endif