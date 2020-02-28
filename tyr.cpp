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
        void onFocus();

};

class Editor : protected Window{
    protected:
    public:
        Editor(int h, int w, int x0, int y0){
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

        void onFocus();
};

class FileViewer : protected Window{
    protected:
    public:
        FileViewer(int h, int w, int x0, int y0){
            Window::width = w;
            Window::height = h;
            Window::win = newwin(Window::height, Window::width, y0,x0);

            wborder(Window::win, 0, 0, 0, 0, 0, 0, 0, 0);

            Window::pan = new_panel(Window::win);
            wrefresh(Window::win);
        }
        Cursor cursor;
        void resize(int, int);

        WINDOW* getWindow(){
            return Window::win;
        }
};

void focusOnFileViewer(FileViewer fs){
    wmove(fs.getWindow(), 1,1);
    waddstr(fs.getWindow(),"Hey whats poppin gamers");
    // when we switch to the file viewer we want to hide the cursor
    curs_set(0);
    // refresh the file viewer window
    wrefresh(fs.getWindow());

}

int main() {
	// sets the locale so that terminals use UTF8 encoding
    //std::setlocale(LC_ALL, "en_US.UTF-8");
	// initializes curses
    initscr();
    // refreshes the screen
    refresh();
    cbreak();
    noecho();
    // creates the editor screen
    Editor ed (LINES-1, COLS-20, 20, 0);
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

