//
// Created by derek on 6/1/20.
// For documentation, see window.cpp
//

#ifndef TYR_WINDOW_H
#define TYR_WINDOW_H

#include <curses.h>
#include <panel.h>


namespace window {
    class Window {
    public:
        int width, height;
        PANEL *pan;
        WINDOW *win;
        PANEL *border_pan;
        WINDOW *border_win;

        virtual void handleInput(int c) = 0;
        virtual void onFocus() = 0;
        virtual void deFocus() = 0;
        virtual void resize(int y, int x) = 0;

        void create_windows(int outer_h, int outer_w, int outer_y0, int outer_x0, int inner_h, int inner_w, int inner_y0, int inner_x0);
        void drawBorder(int attrs);
    };
}

#endif //TYR_WINDOW_H
