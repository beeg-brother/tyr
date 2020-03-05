#include <curses.h>
#include <panel.h>
#include <clocale>
#include <string>

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
            int lineNumLen = 4;//how much space we give the line numbers
            win = newwin(h - 2, w - 2 - lineNumLen, y0 + 1, x0 + 1 + lineNumLen);
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
            cursor = Cursor();
            cursor.x = 0;
            cursor.y = 0;
            mvwaddstr(Window::border_win, cursor.y+1, 1, std::to_string(1).c_str());
        }

        WINDOW* getWindow(){
            return Window::win;
        }

        Cursor cursor;
        void resize(int, int);

        void onFocus();

        void handleInput(int c){
            switch (c){
                case KEY_RIGHT:
                    cursor.x += 1;
                    wmove(Window::win, cursor.y, cursor.x);
                    break;
                case KEY_LEFT:
                    cursor.x -= 1;
                    wmove(Window::win, cursor.y, cursor.x);
                    break;
                case KEY_UP:
                    cursor.y -= 1;
                    wmove(Window::win, cursor.y, cursor.x);
                    break;
                case KEY_DOWN:
                    cursor.y += 1;
                    wmove(Window::win, cursor.y, cursor.x);
                    break;
                case 10: // ENTER KEY
                    cursor.y +=1;
                    cursor.x = 0;
                    wmove(Window::win, cursor.y, cursor.x);
                    mvwaddstr(Window::border_win, cursor.y+1, 1, std::to_string(cursor.y+1).c_str());
                    break;
                case 127: // BACKSPACE KEY
                    cursor.x -=1;
                    wmove(Window::win, cursor.y, cursor.x);
                    break;
                default:
                    waddch(Window::win, c);
                    cursor.x += 1;
                    wmove(Window::win, cursor.y, cursor.x);
                    break;
            }
            wrefresh(Window::border_win);
            wrefresh(Window::win);
        }

        // this is a pointer to an array of pointers. yes its terrible.
        // see: https://www.geeksforgeeks.org/difference-between-pointer-to-an-array-and-array-of-pointers/
        // and: http://www.fredosaurus.com/notes-cpp/newdelete/50dynamalloc.html
        // i'm using a pointer to an array because it allows for dynamic length of the array
        // i'm using an array of pointers because copying pointers from one array to another should be 100x more efficient than copying full strings
        // also, keep this as the last declaration in the class. yes.
        std::string* *strs[];
};

class FileViewer : protected Window{
    protected:
    public:
        FileViewer(int h, int w, int y0, int x0){
            Window::create_windows(h, w, y0, x0);
        }
        Cursor cursor;
        void resize(int, int);

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
            int screen_rows, screen_cols;
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
            Window::create_windows(height + 2, width + 2, start_y - 1, start_x - 1);
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
    // creates the editor screen
    ed = new Editor(LINES-1, COLS-20, 0, 20);
    fs = new FileViewer(LINES-1, 21, 0, 0);
    //Dialog dia ("how's this???? is this enough lines to trigger wrap yet????");
    //update the panel stacking
    update_panels();
    doupdate();
    //focusOnFileViewer(fs);
    mainLoop();
    // close curses
    endwin();
    return 0;
}