//
// Created by derek on 3/10/21.
//

#ifndef TYR_FILEBROWSER_H
#define TYR_FILEBROWSER_H
//TODO: refactor all ifndefs to use TYR_<FILE>_H

#include "constants.h"
#include "window.h"
#include <map>
#include <curses.h>
#include <panel.h>
#include <iostream>
#include <clocale>
#include <string>
#include <menu.h>
#include <filesystem>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <fstream>

namespace fsys = std::filesystem;

namespace filebrowser {
    class FileBrowser : public window::Window {
    public:

        // the list of choices that will show up in the menu
        std::vector <fsys::path> menu_choices;
        // the number of files in the menu choices
        int num_files;
        // the root path, where tyr is opened from
        fsys::path cwd = fsys::current_path();
        // drawing related things:
        // TODO: possibly unnecessary, refactor to use getmaxxy()
        int window_width;
        int window_height;
        int scroll_start = 0;
        // the index of the currently selected item
        int current_index;
        // the theme settings are stored here or something
        std::map<std::string, int> color_map;

        // constructors
        FileBrowser(int h, int w, int y0, int x0, std::map<std::string, int> c_map);

        FileBrowser(int h, int w, int y0, int x0, std::map<std::string, int> c_map,
                    std::vector <fsys::path> choices);

        // Color System functions
        void init_color_pairs();

        void setColorMap(std::map<std::string, int> map);

        // draws the menu onto whatever window is currently selected
        void drawMenu();

        // removes the menu from whatever window is currently selected
        void removeMenu();

        // gets files from the given directory as a vector of paths
        std::vector <fsys::path> getDirFiles(fsys::path path);

        // updates the menu on the screen
        // takes in the new elements and the position in which they are supposed to be put in
        void updateMenu(std::vector <fsys::path> new_elements, int index);

        // changes the vector of paths that the menu uses
        void setMenuItems(std::vector <fsys::path> path_vector);


        // collapses the directory located in the menu at the given index
        void collapseDirectory(int directoryIndex);

        // expands the directory located at the index in the menu
        void expandDirectory(int directoryIndex);

        // sets the user selection to the path at the index provided
        void setCurrentItem(int index);

        // returns the vector of all the current paths in the menu
        std::vector <fsys::path> getMenuItems();

        // returns the path of the currently selected item
        fsys::path getCurrentItem();

        // returns the index of the path specified
        int getIndexOf(std::string path);

        // moves the menu down 1 slot
        void menu_down();

        // moves the menu up 1 slot
        void menu_up();

        // collapses the selected item if its a directory
        void menu_left();

        // expands the selected item if its a directory
        void menu_right();

        // scrolls the menu items to fit the current item on the screen
        void scrollToFit();


        // normal Window functions
        void resize(int, int);
        void create_windows(int h, int w, int y0, int x0);
        WINDOW *getWindow();
        void onFocus();
        void deFocus();
        void handleInput(int c);
    };
}


#endif //TYR_FILEBROWSER_H
