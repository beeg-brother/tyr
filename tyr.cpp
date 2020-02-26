#include <curses.h>

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
            waddch(stdscr, 'a');
            wmove(Window::win, 1,1);
            wborder(stdscr, 0, 0, 0, 0, 0, 0, 0, 0);
            waddch(stdscr, 'a');
            wrefresh(Window::win);
            wrefresh(stdscr);
        }
        Cursor cursor;
        void resize(int, int);
};

int main() {
    initscr();
    cbreak();
    noecho();
    Editor ed (10, 10, 0, 0);
    getch();
    endwin();
    return 0;
}