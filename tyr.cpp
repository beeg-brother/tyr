#include <curses.h>
#include <panel.h>
#include <clocale>

struct Cursor {
    int x, y;
};


class Window {
    protected:
        int width, height;
        WINDOW *win;
        PANEL *pan;
    public:
        void resize(int, int);

};

class Editor : protected Window{
    protected:
    public:
        Editor(int w, int h, int x0, int y0){
            Window::width = w;
            Window::height = h;
            Window::win = newwin(Window::height, Window::width, y0, x0);
            //draws the border around the window
            wborder(Window::win, 0, 0, 0, 0, 0, 0, 0, 0);
            // make the panel that the window is attached to:
            Window::pan = new_panel(Window::win);
            // refresh the window
            wrefresh(Window::win);
            
        }
        Cursor cursor;
        void resize(int, int);
};

int main() {
	// sets the locale so that terminals use UTF8 encoding
	//std::setlocale(LC_ALL, "en_US.UTF-8");
	std::setlocale(LC_ALL, "en_US.UTF-8");
	// initializes curses
    initscr();
    // refreshes the screen
    refresh();
    cbreak();
    noecho();
    // creates the editor screen
    Editor ed (COLS-20, LINES - 1, 20, 0);
    getch();
    // close curses
    endwin();
    return 0;
}