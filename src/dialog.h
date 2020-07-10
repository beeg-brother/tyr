//
// Created by derek on 6/1/20.
//

#ifndef TYR_DIALOG_H
#define TYR_DIALOG_H

#include "constants.h"
#include "window.h"

#include <vector>
#include <string>
#include <memory>
#include <curses.h>

namespace dialog {

    void draw_box(WINDOW* win, int attrs, int y1, int x1, int y2, int x2);

    // TODO: make only some kinds of DialogElements focusable
    class DialogElement {
    public:

        DialogElement();

        bool isFocused;

        virtual void handleInput(int c) = 0;

        virtual void refresh(WINDOW *win, int startY) = 0;

        virtual void onFocus() = 0;

        virtual void deFocus() = 0;

        virtual int requestNumLines(int width) = 0;

        virtual void placeCursor(WINDOW *win, int startY) = 0;
    };

    class InputElement : public DialogElement {
    public:
        std::string contents;
        int numLines;
        int position;

        InputElement(int lines);

        void onFocus();

        void deFocus();

        int requestNumLines(int width);

        void refresh(WINDOW *win, int startY);

        void placeCursor(WINDOW *win, int startY);

        void handleInput(int c);
    };

    class ButtonsElement : public DialogElement {
    public:
        std::vector <std::string> options;
        std::vector<std::string>::iterator selected;
        short numOptions;
        bool eos;
        short intendedYLevel;

        ButtonsElement(std::vector <std::string> opts, bool exitOnSelect);

        void onFocus();

        void deFocus();

        int requestNumLines(int width);

        void refresh(WINDOW *win, int startY);

        void placeCursor(WINDOW *win, int startY);

        void handleInput(int c);
    };

    class StringElement : public DialogElement {
    public:
        const char *message;
        int length;

        StringElement(const char *str, int len);

        ~StringElement();

        void onFocus();

        void deFocus();

        int requestNumLines(int width);

        void refresh(WINDOW *win, int yval);

        void placeCursor(WINDOW *win, int startY);

        void handleInput(int c);
    };

    class Dialog : public window::Window {
    protected:
        std::vector <std::shared_ptr<DialogElement>> elements;
        std::vector<std::shared_ptr < DialogElement>>::
        iterator currentElement;
        std::vector<int> cached_startYs_cumulative;

    public:
        Dialog(std::vector <std::shared_ptr<DialogElement>> els);

        void reloadHeights();

        void refresh();

        void handleInput(int c);

        void resize(int, int);

        void onFocus();

        void deFocus();
    };
}

#endif //TYR_DIALOG_H
