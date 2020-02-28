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
            win = newwin(h - 2, w - 2, y0 + 1, x0 + 1);
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
        }
        Cursor cursor;
        void resize(int, int);

        void onFocus();
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
            
        }
};

void focusOnFileViewer(FileViewer fs){
    waddstr(fs.getWindow(),"Hey whats poppin gamers");
    // when we switch to the file viewer we want to hide the cursor
    curs_set(0);
    // refresh the file viewer window
    wrefresh(fs.getWindow());

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
    // creates the editor screen
    Editor ed (LINES-1, COLS-20, 0, 20);
    FileViewer fs (LINES-1, 21, 0, 0);
    //update the panel stacking
    update_panels();
    doupdate();
    focusOnFileViewer(fs);
    getch();
    // close curses
    endwin();
    return 0;
}

