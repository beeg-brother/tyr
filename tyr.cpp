#include <curses.h>
#include <clocale>

struct Cursor {
    int x, y;
};


class Window {
    protected:
        int width, height;
        WINDOW *win;
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
            wborder(Window::win, 0, 0, 0, 0, 0, 0, 0, 0);
            wrefresh(Window::win);
            
        }
        Cursor cursor;
        void resize(int, int);
};

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
    Editor ed (10, 10, 1, 1);

    getch();
    // close curses
    endwin();
    return 0;
}