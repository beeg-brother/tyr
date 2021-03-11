//
// Created by derek on 3/10/21.
//

#include "filebrowser.h"

namespace fsys = std::filesystem;

namespace filebrowser {
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

    FileBrowser::FileBrowser(int h, int w, int y0, int x0, std::map<std::string, int> c_map) {
        create_windows(h, w, y0, x0);
        getmaxyx(win, window_height, window_width);
        fsys::path cwd = fsys::current_path();
        setMenuItems(getDirFiles(cwd));
        color_map = c_map;
    }

    FileBrowser::FileBrowser(int h, int w, int y0, int x0, std::map<std::string, int> c_map,
                             std::vector <fsys::path> choices) {
        FileBrowser(h, w, y0, x0, c_map);
        menu_choices = choices;
        num_files = menu_choices.size();
    }

    //TODO: this is possibly extraneous, tyr should init all the color pairs on load of config file
    void FileBrowser::init_color_pairs(){
        start_color();

        init_pair(directoriesColor, color_map["directories"], -1);
        init_pair(filesColor, color_map["files"], -1);
        init_pair(selectedMenuItemColor,color_map["selectedItem"], -1);
    }

    void FileBrowser::setColorMap(std::map<std::string,int> map){
        color_map = map;
    }

    // TODO: possibly migrate this to 2 functions: generating the menu and actually writing it
    //  that way, the generation doesn't have to happen as frequently

    // draws the menu onto the screen
    void FileBrowser::drawMenu(){
        init_color_pairs();
        werase(win);
        num_files = menu_choices.size();

        // check to make sure that there is a window that the menu is attached to
        if (win != nullptr){
            scrollToFit();
            //TODO: migrate this to onFocus and deFocus
            curs_set(0);
            for (int currenty = scroll_start; currenty < scroll_start + window_height; currenty++){
                if (currenty < num_files){

                    // count how "deep" of a file it is - how many parent folders are there
                    // for each one, write a line in front of the file name
                    // this does overcount by one, but it will be overwritten later so no worries
                    int prepends = 0;
                    if (menu_choices[currenty].parent_path() != cwd){
                        fsys::path temp = menu_choices[currenty];
                        while(temp.parent_path() != cwd){
                            temp = temp.parent_path();
                            mvwaddch(win, currenty - scroll_start, prepends, ACS_VLINE);
                            prepends++;
                        }
                    }

                    if(prepends != 0){
                        // this first test makes sure not to do index out of bounds later
                        // basically if this is the last item in the list there's no need to have a T
                        if(prepends == menu_choices.size() - 1){
                            mvwaddch(win, currenty-scroll_start, prepends - 1, ACS_LLCORNER);
                        } else {
                            // if the next element is a subfile or is in the same folder as the current element, put a T
                            // otherwise, put a corner
                            if (menu_choices[currenty + 1].parent_path() == menu_choices[currenty].parent_path() |
                                menu_choices[currenty + 1].parent_path() == menu_choices[currenty]){
                                mvwaddch(win, currenty - scroll_start, prepends - 1, ACS_LTEE);
                            } else {
                                mvwaddch(win, currenty - scroll_start, prepends - 1, ACS_LLCORNER);
                            }
                        }
                    }

                    // if the item is a directory
                    if (fsys::is_directory(menu_choices[currenty])){
                        // directories are underlined, dimmed, and the color chosen in the theme
                        wattrset(win, A_UNDERLINE | A_DIM | COLOR_PAIR(directoriesColor));
                    }
                        // otherwise its just a file
                    else {
                        // files are dimmed and the color chosen for them in the theme
                        wattrset(win, A_DIM | COLOR_PAIR(filesColor));
                    }
                    // if its selected
                    if (currenty == current_index){
                        wattrset(win, A_BOLD | COLOR_PAIR(selectedMenuItemColor));
                    }


                    //add the item to the menu
                    mvwaddnstr(win, currenty - scroll_start, prepends, menu_choices[currenty].filename().u8string().c_str(),window_width - prepends);
                    //reset the text attributes
                    wstandend(win);
                }
            }
        }
        wrefresh(win);
    }

    //removes the menu from the screen
    void FileBrowser::removeMenu(){
        // check to make sure that there is a window that the menu is attached to
        if (win != nullptr){
            werase(win);
        }
    }

    // gets files from the given directory as a vector of paths
    std::vector<fsys::path> FileBrowser::getDirFiles(fsys::path path){
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
    void FileBrowser::updateMenu(std::vector<fsys::path> new_elements, int index){
        // insert the elements into the menu_choices in the correct index
        menu_choices.insert(menu_choices.begin() + index + 1,new_elements.begin(), new_elements.end());
        num_files = menu_choices.size();
    }

    // changes the vector of paths that the menu uses
    void FileBrowser::setMenuItems(std::vector<fsys::path> path_vector){
        menu_choices = path_vector;
        num_files = path_vector.size();
    }

    // collapses the directory located in the menu at the given index
    void FileBrowser::collapseDirectory(int directoryIndex){
        bool collapseNecessary = false;

        if(directoryIndex == menu_choices.size() - 1){
            // if this is the last item in the list, don't bother collapsing
            collapseNecessary = false;
        } else {
            if(menu_choices[directoryIndex + 1].parent_path() == menu_choices[directoryIndex]){
                // if the next item is a subfile, collapse is necessary
                collapseNecessary = true;
            }
        }
        // if the directory has been expanded

        if (collapseNecessary){
            // get every single file in the directory
            std::vector<fsys::path> all_files;
            for(auto& p: fsys::recursive_directory_iterator(menu_choices[directoryIndex])){
                all_files.push_back(p.path());
            }
            // remove every occurrence of each subpath from the menu
            for (int i = 0; i < all_files.size(); i++){
                // remove it from the menu choices
                menu_choices.erase(std::remove(menu_choices.begin(), menu_choices.end(), all_files[i]), menu_choices.end());
            }
        }
        removeMenu();
        wrefresh(win);
        drawMenu();
    }

    // expands the directory located at the index in the menu
    void FileBrowser::expandDirectory(int directoryIndex){

        bool expansionNecessary = false;

        if(directoryIndex == menu_choices.size() - 1){
            // if this is the last element in the list, expand (this is done to avoid out of bounds with the next test)
            expansionNecessary = true;
        } else {
            if(menu_choices[directoryIndex + 1].parent_path() == menu_choices[directoryIndex]){
                // if the next element is a subfile, don't expand again
                expansionNecessary = false;
            } else {
                // if the next element is not a subfile, expand
                expansionNecessary = true;
            }
        }
        if(expansionNecessary){
            fsys::path path_expanding = menu_choices[directoryIndex];
            std::vector<fsys::path> new_elements;
            new_elements = getDirFiles(path_expanding);
            removeMenu();
            wrefresh(win);
            updateMenu(new_elements, directoryIndex);
            drawMenu();
        }
    }

    // sets the user selection to the path at the index provided
    void FileBrowser::setCurrentItem(int index){
        current_index = index;
    }

    // returns the vector of all the current paths in the menu
    std::vector<fsys::path> FileBrowser::getMenuItems(){
        return menu_choices;
    }

    // returns the path of the currently selected item
    fsys::path FileBrowser::getCurrentItem(){
        return menu_choices[current_index];
    }

    // returns the index of the path specified
    // TODO: seems inefficient
    int FileBrowser::getIndexOf(std::string path){
        for(int i = 0; i < num_files; i++){
            if (menu_choices[i].u8string() == path){
                return i;
            }
        }
        return -1;
    }

    void FileBrowser::menu_down(){
        if (current_index == num_files - 1){
            current_index = 0;
        }

        else{
            current_index += 1;
        }
        removeMenu();
        drawMenu();
    }
    void FileBrowser::menu_up(){
        if (current_index == 0){
            current_index = num_files -1;
        }
        else {
            current_index -= 1;
        }
        removeMenu();
        drawMenu();
    }
    void FileBrowser::menu_left(){
        // if its a directory, collapse it
        if (fsys::is_directory(menu_choices[current_index])){
            collapseDirectory(current_index);
        }
        wrefresh(win);
    }
    void FileBrowser::menu_right(){
        // if its a directory expand it
        if (fsys::is_directory(menu_choices[current_index])){
            expandDirectory(current_index);

        }
        wrefresh(win);
    }

    // scrolls the menu items to fit the current item on the screen
    void FileBrowser::scrollToFit(){
        // if the current index is outside of the range
        // we need to scroll to fit the current selection onto the screen
        if (current_index < scroll_start){
            scroll_start -= scroll_start - current_index;
            wrefresh(win);
        }
        if (current_index > scroll_start + window_height - 1){
            scroll_start += current_index - (scroll_start + window_height - 1);
            wrefresh(win);
        }

    }

    // TODO: resizing
    void FileBrowser::resize(int, int) {
        return;
    };

    void FileBrowser::create_windows(int h, int w, int y0, int x0) {
        Window::create_windows(h, w, y0, x0, h - 2, w - 2, y0 + 1, x0 + 1);
    }

    WINDOW* FileBrowser::getWindow() {
        return Window::win;
    }

    void FileBrowser::onFocus() {
        return;
    }

    void FileBrowser::deFocus() {
        return;
    }

    void FileBrowser::handleInput(int c) {
        switch (c) {
            case KEY_DOWN:
                menu_down();
                break;
            case KEY_UP:
                menu_up();
                break;
            case KEY_RIGHT:
                menu_right();
                break;
            case KEY_LEFT:
                menu_left();
                break;
        }
    }


}