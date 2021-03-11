//
// Created by derek on 6/1/20.
//

#include "window.h"
#include "constants.h"
#include <curses.h>
#include <panel.h>



namespace window {
    // A wrapper class to allow for easy stacking of windows to properly simulate a GUI in the CLI
    // Uses panels to achieve this
    // Each window has a border. To do this, a separate border window and panel are used.
    // Subclasses are allowed to define the way the outer border offset works. See create_windows.

    /*
    // width and height of the window, but these aren't properly updated yet (resize not yet implemented)
    int width, height;
    // the PANEL the window resides in, allowing for easy stacking
    PANEL *pan;
    // the WINDOW in use
    WINDOW *win;
    // the PANEL the outer border resides in
    PANEL *border_pan;
    // the WINDOW the outer border is written to
    WINDOW *border_win;
     */

    // handles any input characters from the user
    // certain characters are intercepted by the top-level logic:
    //     * control+tab
    //     * possbily more
    // c: the character the user input
    // virtual void handleInput(int c) = 0;

    // any actions that need to be taken when the user switches focus to the window
    // for example, changing the border color, updating the contents of the window
    // virtual void onFocus() = 0;

    // any actions that need to be taken when the user switches focus away from the window
    // for example, changing the border color
    // virtual void deFocus() = 0;

    // resize the panels and windows
    // this is designed to be used for when the terminal changes sizes, but other uses are allowed
    // y: the intended height of the windows
    // x: the intended width of the windows
    // virtual void resize(int y, int x) = 0;

    // this method should be called by subclass methods, not called directly
    // this creates the two windows and panels, but allows for customization of the offset of the border
    // this allows for use of the border panel: for example, the line numbers in the Editor subclass are written to the border window
    // outer_h: the height of the border window
    // outer_w: the width of the border window
    // outer_y0: the y-value of the upper left corner of the border window
    // outer_x0: the x-value of the upper left corner of the border window
    // extrapolate for the other 4 arguments
    void Window::create_windows(int outer_h, int outer_w, int outer_y0, int outer_x0, int inner_h, int inner_w, int inner_y0, int inner_x0) {
        width = outer_w;
        height = outer_h;
        border_win = newwin(outer_h, outer_w, outer_y0, outer_x0);
        drawBorder(COLOR_PAIR(borderFocusedColor));
        border_pan = new_panel(border_win);
        win = newwin(inner_h, inner_w, inner_y0, inner_x0);
        pan = new_panel(win);
        wrefresh(border_win);
        wrefresh(win);
    }

    // this draws the border of the window, allowing for attributes to be specified
    // this draws only onto border_win
    void Window::drawBorder(int attrs) {
        wborder(border_win, (ACS_VLINE | attrs),
                (ACS_VLINE | attrs),
                (ACS_HLINE | attrs),
                (ACS_HLINE | attrs),
                (ACS_ULCORNER | attrs),
                (ACS_URCORNER | attrs),
                (ACS_LLCORNER | attrs),
                (ACS_LRCORNER | attrs));
    }

    // TODO: create place_cursor method?
}