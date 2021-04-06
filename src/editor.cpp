//
// Created by derek on 6/1/20.
//

#include "constants.h"
#include "cursor.h"
#include "editor.h"
#include "window.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

namespace editor {

    /*
    // TODO: rewrite to use a list instead of a vector
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

    std::filesystem::path filepath;
    */


    Editor::Editor(int h, int w, int y0, int x0){
        create_windows(h, w, y0, x0);
        cursor = Cursor();
        cursor.screen_x = 0;
        cursor.screen_y = 0;
        strs.push_back(std::string());
        scroll_offset = 0;
        getmaxyx(win, window_height, window_width);
        rewrite();
    }

    Editor::Editor(int h, int w, int y0, int x0, std::filesystem::path filename){
        create_windows(h, w, y0, x0);
        cursor = Cursor();
        cursor.screen_x = 0;
        cursor.screen_y = 0;
        filepath = filename;
        std::string temp_string;
        std::ifstream file(filepath.c_str());
        while(std::getline(file, temp_string)){
            strs.push_back(temp_string);
        }
        scroll_offset = 0;
        getmaxyx(win, window_height, window_width);
        rewrite();
    }

    // create editor windows, leaving room for line numbering.
    void Editor::create_windows(int h, int w, int y0, int x0){
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
    WINDOW* Editor::getEditorWindow(){
        return Window::win;
    }

    // returns the PANEL object that the editing window is in
    PANEL* Editor::getEditorPanel(){
        return Window::pan;
    }

    // TODO: resizing
    void Editor::resize(int, int){
        return;
    };

    void Editor::rewrite_line_nums(){
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

        wattron(border_win, COLOR_PAIR(lineNumbersColor));

        for(int i = scroll_offset + 1; i <= scroll_offset + max; i++){
            // create a right-padded string for the line number
            std::sprintf(out, "%*d", line_num_width, i);
            //TODO: line numbers are NOT the right color for some reason
            mvwaddnstr(Window::border_win, i, 1, out, line_num_width);
        }

        wattroff(border_win, COLOR_PAIR(lineNumbersColor));

        // no mem-leaks pls
        delete out;
        wrefresh(border_win);
    }

    // redraws all strings after clearing the screen
    void Editor::rewrite(){
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

    void Editor::onFocus(){
        return;
    }

    void Editor::deFocus(){
        return;
    }

    // deals with the input of characters to the editor.
    // typing, arrow keys, etc.
    void Editor::handleInput(int c){
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
                    cursor.screen_y = std::min(cursor.line_num, getmaxy(Window::win));
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
            case 127:
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
                    wclrtoeol(win);
                    cursor.screen_x -= 1;
                    cursor.line_position -= 1;
                    // clear the line, then redraw it to update all characters
                    wattron(win, COLOR_PAIR(textColor));
                    mvwaddnstr(win, cursor.screen_y, cursor.screen_x, strs[cursor.line_num].substr(cursor.line_position).data(), window_width);
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
}