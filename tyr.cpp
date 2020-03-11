#include <curses.h>
#include <panel.h>
#include <clocale>
#include <string>


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
        void create_windows(int h, int w, int y0, int x0){
            width = w;
            height = h;
            int lineNumLen = 4; //how much space we give the line numbers
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
        // number of elements in strs
        // if this is 0, strs is empty
        // this means that strs[strs_size] is out of bounds
        // strs[strs_size - 1] is the last element
        int strs_size;

        Editor(int h, int w, int y0, int x0){
            Window::create_windows(h, w, y0, x0);
            cursor = Cursor();
            cursor.screen_x = 0;
            cursor.screen_y = 0;
            mvwaddstr(Window::border_win, cursor.screen_y+1, 1, std::to_string(1).c_str());
            *strs = new std::string*[1];
            strs_size = 1;
            *strs[0] = new std::string();
        }

        WINDOW* getEditorWindow(){
            return Window::win;
        }

        PANEL* getEditorPanel(){
            return Window::pan;
        }

        Cursor cursor;
        void resize(int, int);

        void onFocus();

        void handleInput(int c){
            switch (c){
                case KEY_RIGHT:
                    if((*(*strs)[cursor.line_num]).size() > cursor.screen_x){ // check if its valid to move over a character
                        cursor.screen_x += 1;
                        cursor.line_position += 1;
                        wmove(Window::win, cursor.screen_y, cursor.screen_x);
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
                    if(cursor.line_position == 0){
                        if(cursor.line_num != 0){
                            cursor.line_position = (*(*strs)[cursor.line_num]).size() - 1;
                            cursor.line_num -= 1;
                            cursor.screen_x = std::min(screen_cols, cursor.line_position);
                        }
                    } else {
                        cursor.line_position -= 1;
                        cursor.screen_x -= 1;
                    }
                    wmove(Window::win, cursor.screen_y, cursor.screen_x);
                    break;
                case KEY_UP:
                    if(cursor.line_num != 0){
                        cursor.line_num -= 1;
                        cursor.line_position = std::min((int)(*(*strs)[cursor.line_num]).size() - 1, cursor.line_position);
                        cursor.screen_y -= 1;
                        cursor.screen_x = std::min(screen_cols, cursor.line_position);
                    } else if (cursor.line_position != 0){
                        cursor.line_position = 0;
                        cursor.screen_x = 0;
                    }
                    wmove(Window::win, cursor.screen_y, cursor.screen_x);
                    break;
                case KEY_DOWN:
                    if(cursor.line_num == strs_size - 1){ // on last line
                        if(cursor.line_position != (*(*strs)[cursor.line_num]).size() - 1){
                            cursor.line_position = (*(*strs)[cursor.line_num]).size() - 1;
                            cursor.screen_x = std::min((int) (*(*strs)[cursor.line_num]).size() - 1, screen_cols);
                        }
                    } else {
                        cursor.line_num += 1;
                        cursor.screen_y = std::min(cursor.line_num, screen_rows);
                        cursor.line_position = std::min(cursor.line_position, (int) (*(*strs)[cursor.line_num]).size() - 1);
                        cursor.screen_x = std::min(cursor.line_position, screen_cols);
                    }
                    wmove(Window::win, cursor.screen_y, cursor.screen_x);
                    break;
                case 10: // ENTER KEY
                    cursor.screen_y += 1;
                    cursor.screen_x = 0;
                    // TODO: add string array copying, inserting new line
                    wmove(Window::win, cursor.screen_y, cursor.screen_x);
                    mvwaddstr(Window::border_win, cursor.screen_y+1, 1, std::to_string(cursor.screen_y+1).c_str());
                    break;
                case 127: // BACKSPACE KEY
                    cursor.screen_x -=1;
                    // TODO: remove previous letter in the active string
                    wmove(Window::win, cursor.screen_y, cursor.screen_x);
                    break;
                default:
                    (*(*strs)[cursor.line_num]).insert(cursor.line_position, 1, (char) c);
                    const char* a = (*(*strs)[cursor.line_num]).data();
                    mvwaddnstr(Window::border_win, 3, 0, std::to_string(cursor.screen_y).data(), 1);
                    mvwaddnstr(Window::border_win, 4, 0, std::to_string(cursor.screen_x).data(), 1);
                    wrefresh(Window::border_win);
                    mvwaddnstr(Window::win, cursor.screen_y+1, cursor.screen_x+1, (*(*strs)[cursor.line_num]).data(), 1);
                    wrefresh(Window::win);
                    mvwaddch(Window::border_win, 0, 0, 'b');
                    wrefresh(Window::border_win);
                    cursor.screen_x += 1;
                    cursor.line_position += 1;
                    wmove(Window::win, cursor.screen_y, cursor.screen_x);
                    break;
            }
            
            //mvwaddstr(Window::win, cursor.screen_y, cursor.screen_x, (*(*strs)[cursor.line_num]).data());
            //wmove(Window::win, cursor.screen_y, cursor.screen_x);
            wrefresh(Window::border_win);
            wrefresh(Window::win);
        }

        // this is a pointer to an array of pointers. yes its terrible.
        // see: https://www.geeksforgeeks.org/difference-between-pointer-to-an-array-and-array-of-pointers/
        // and: http://www.fredosaurus.com/notes-cpp/newdelete/50dynamalloc.html
        // i'm using a pointer to an array because it allows for dynamic length of the array
        // i'm using an array of pointers because copying pointers from one array to another should be 100x
        // more efficient than copying full strings
        // to access an element of this shit, use (*(*strs)[n]). im sorry.
        // i can resize strings nicely:
        // see http://www.cplusplus.com/reference/string/string/insert/
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
    getmaxyx(stdscr, screen_rows, screen_cols);
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