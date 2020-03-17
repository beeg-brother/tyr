#include <curses.h>
#include <panel.h>
#include <clocale>
#include <string>
#include <vector>

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

int screen_rows, screen_cols;

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
        // this method should be called by subclass methods, not called directly
        void create_windows(int outer_h, int outer_w, int outer_y0, int outer_x0, int inner_h, int inner_w, int inner_y0, int inner_x0){
            width = outer_w;
            height = outer_h;
            border_win = newwin(outer_h, outer_w, outer_y0, outer_x0);
            wborder(border_win, 0, 0, 0, 0, 0, 0, 0, 0);
            border_pan = new_panel(border_win);
            win = newwin(inner_h, inner_w, inner_y0, inner_x0);
            pan = new_panel(win);
            wrefresh(border_win);
            wrefresh(win);
        }

};

class Editor : protected Window{
    protected:
    public:

        std::vector<std::string> strs;
        // TODO: implement scrolling
        int scroll_offset;
        int window_width;
        int window_height;

        Editor(int h, int w, int y0, int x0){
            create_windows(h, w, y0, x0);
            cursor = Cursor();
            cursor.screen_x = 0;
            cursor.screen_y = 0;
            mvwaddstr(Window::border_win, cursor.screen_y+1, 1, std::to_string(1).c_str());
            strs.push_back(std::string());
            scroll_offset = 0;
            getmaxyx(win, window_height, window_width);
        }

        // create editor windows, leaving room for line numbering.
        void create_windows(int h, int w, int y0, int x0){
            int line_num_width = 0;
            int num_rows = screen_rows;
            // calculate the number of digits in the maximum line number
            while (num_rows){
                num_rows /= 10;
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

        Cursor cursor;

        void resize(int, int);

        void onFocus();

        void rewrite(){
            // TODO: rewrite the line numbers
            wclear(win);
            for(int i = 0; i < std::min(screen_rows, (int) strs.size()); i++){
                mvwaddnstr(win, i, 0, strs[i].data(), window_width);
            }
            wrefresh(win);
        }

        // deals with the input of characters to the editor.
        // typing, arrow keys, etc.
        void handleInput(int c){
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

                    // TODO: having this here is hacky, change it
                    mvwaddstr(Window::border_win, cursor.screen_y+1, 1, std::to_string(cursor.screen_y+1).c_str());
                    break;
                case 127: // BACKSPACE KEY
                    cursor.screen_x -=1;
					cursor.line_position -= 1;
                    // TODO: make the backspace remove chars from the display that aren't being overwritten
                    // don't use rewrite(), computationally expensive tbh
					strs[cursor.line_num].erase(cursor.line_position, 1);
                    wmove(Window::win, cursor.screen_y, cursor.screen_x);
                    break;
                default:
                    strs[cursor.line_num].insert(cursor.line_position, 1, (char) c);
                    const char* a = strs[cursor.line_num].data();
                    mvwaddnstr(Window::win, cursor.screen_y, 0, strs[cursor.line_num].data(), window_width);
                    wrefresh(Window::win);
                    cursor.screen_x += 1;
                    cursor.line_position += 1;
                    wmove(Window::win, cursor.screen_y, cursor.screen_x);
                    break;
            }
            //mvwaddstr(Window::win, cursor.screen_y, cursor.screen_x, strs[cursor.line_num].data());
            // update cursor position
            wmove(Window::win, cursor.screen_y, cursor.screen_x);
            wrefresh(Window::border_win);
            wrefresh(Window::win);
        }
};

class FileViewer : protected Window{
    protected:
    public:
        FileViewer(int h, int w, int y0, int x0){
            create_windows(h, w, y0, x0);
        }
        Cursor cursor;
        void resize(int, int);

        void create_windows(int h, int w, int y0, int x0){
            Window::create_windows(h, w, y0, x0, h - 2, w - 2, y0 + 1, x0 + 1);
        }

        WINDOW* getWindow(){
            return Window::win;
        }
};

class Dialog : protected Window{
    protected:
    public:
        Dialog(std::string str){
            int len = str.size();
            // get the size of stdscr (fills terminal, so basically get terminal size)
            getmaxyx(stdscr, screen_rows, screen_cols);
            //screen_rows = LINES;
            //screen_cols = COLS;
            // assuming that we want the max width to be 1/3 of the screen (idk i just picked this #)
            int width = std::min(len, (screen_cols + 2)/3); // this looks jank but stack overflow says this will round the division up
            // determine the number of rows
            int height;
            if (len < width){
                height = 1;
            } else{
                height = len/width + (int) (len % width != 0);
            }
            // center dialog box
            int start_x = (screen_cols + 1)/2 - (width + 1)/2; // again, rounding up the division
            int start_y = (screen_rows + 1)/2 - (height + 1)/2;
            // offsets to account for the border
            Window::create_windows(height + 2, width + 2, start_y - 1, start_x - 1, height, width, start_y, start_x);
            waddstr(Window::win, str.data());
            wrefresh(Window::win);
        }
};

Editor *ed;
FileViewer *fs;


void focusOnFileViewer(FileViewer fs){
    waddstr(fs.getWindow(),"Hey whats poppin gamers");
    // when we switch to the file viewer we want to hide the cursor
    curs_set(0);
    // refresh the file viewer window
    wrefresh(fs.getWindow());
}

void mainLoop(){
    int c;
    while(1){
        c = getch();
        ed->handleInput(c);
    };
}

int main() {
	// sets the locale so that terminals use UTF8 encoding
    std::setlocale(LC_ALL, "en_US.UTF-8");
	// initializes curses
    initscr();
    // refreshes the screen
    refresh();
    cbreak();
    noecho();
    keypad(stdscr, true);
    getmaxyx(stdscr, screen_rows, screen_cols);
    // creates the editor screen
    ed = new Editor(screen_rows-1, screen_cols-20, 0, 20);
    fs = new FileViewer(screen_rows-1, 21, 0, 0);
    //Dialog dia ("how's this???? is this enough lines to trigger wrap yet????");
    //update the panel stacking
    //top_panel(ed->getEditorPanel());
    update_panels();
    doupdate();
    //focusOnFileViewer(fs);
    mainLoop();
    // close curses
    endwin();
    return 0;
}