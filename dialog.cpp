//
// Created by derek on 6/8/20.
//

#include "constants.h"
#include "window.h"
#include <vector>
#include <memory>
#include <string>

namespace dialog {

    // draw a border using line graphics to the rectangle described by the points (x1, y1) and (x2, y2)
    // (x1, y1) is higher and more to the left than (x2, y2), swapping this order breaks it
    // note that as is customary with curses, y-values come first in the argument list
    // win: the WINDOW to draw to
    // attrs: any attributes to combine with the line characters (namely colors)
    // y1: y-value of the upper left point
    // x1: x-value of the upper left point
    // y2: y-value of the lower right point
    // x2: x-value of the lower right point
    void draw_box(WINDOW* win, int attrs, int y1, int x1, int y2, int x2){
        int h = y2 - y1;
        int w = x2 - x1;
        wmove(win, y1, x1);
        wvline(win, ACS_VLINE | attrs, h);
        waddch(win, ACS_ULCORNER | attrs);
        whline(win, ACS_HLINE | attrs, w - 1);
        wmove(win, y1, x2);
        wvline(win, ACS_VLINE | attrs, h);
        waddch(win, ACS_URCORNER | attrs);
        wmove(win, y2, x1);
        whline(win, ACS_HLINE | attrs, w);
        waddch(win, ACS_LLCORNER | attrs);
        wmove(win, y2, x2);
        waddch(win, ACS_LRCORNER | attrs);
    }


// TODO: make only some kinds of DialogElements focusable
    class DialogElement {
    public:
        // is the user focused on this element?
        bool isFocused = false;

        // handles input characters
        // some characters are intercepted by the Dialog class:
        //     * tab / shift+tab
        //     * up / down
        // that's all so far
        // c: the character needing to be handled
        virtual void handleInput(int c) = 0;

        // refreshes the DialogElement on the window
        // win: the window the Dialog is painted on
        // startY: the first y-value the DialogElement is allowed to use
        //         elements can use from startY to startY+requestNumLines(width)
        //         painting outside of this area will be overwritten if there is another element below
        //         elements can use the entire width of the window
        virtual void refresh(WINDOW* win, int startY) = 0;

        // any actions that need to be done when the user focuses on the element
        // for example: changing cursor settings (visible/invis), changing border colors to look focused, etc
        virtual void onFocus() = 0;

        // any actions that need to be done as the user switches focus away from the element
        // for example: resetting cursor settings, changing border colors to look unfocused, etc
        virtual void deFocus() = 0;

        // this allows the DialogElement to reserve space in the window based on a width provided
        // note that this does not guarantee that the next refresh() will use a window with this width
        // storing width in the DialogElement is highly discouraged as it is generally pointless
        // width: the size of the hypothetical window
        virtual int requestNumLines(int width) = 0;

        // this is called on the focused element of a Dialog window on each refresh
        // it allows the DialogElement to place the cursor where it needs to be for the Dialog appearance
        // this basically only applies for the InputElement, but it might be useful in the future
        // note that it is perfectly valid behavior to just return if the cursor is invisible anyway - efficiency
        // win: the window the Dialog is painted on
        // startY: the first y-value that the DialogElement was allowed to draw in (see refresh() )
        virtual void placeCursor(WINDOW* win, int startY) = 0;
    };

    class InputElement : public DialogElement{
    public:
        std::string contents;
        int numLines;
        int position;
        // TODO: implement horizontal scrolling

        InputElement(int lines){
            // TODO: implement lines
            numLines = lines;
            position = 0;
        }

        void onFocus(){
            isFocused = true;
            curs_set(1);
            return;
        }

        void deFocus(){
            isFocused = false;
            curs_set(1);
            return;
        }

        int requestNumLines(int width){
            // TODO: implement lines
            return 3;
        }

        void refresh(WINDOW* win, int startY){
            draw_box(win, COLOR_PAIR(borderFocusedColor), startY, 0, startY + 2, getmaxx(win) - 1);
//            attron(COLOR_PAIR(textColor));
            mvwaddnstr(win, startY + 1, 1, contents.data(), getmaxx(win) - 2);
//            attroff(COLOR_PAIR(textColor));
            wmove(win, startY + 1, position + 1);
        }

        void placeCursor(WINDOW* win, int startY){
            wmove(win, startY+1, std::min(getmaxx(win) - 2, position + 1));
        }

        void handleInput(int c){
            // TODO: move cursor to proper position
            //logMessage("InputElement received " + std::to_string(KEY_BACKSPACE));
            switch(c){
                case KEY_RIGHT :
                    // TODO: bounding based on window, moving cursor
                    position++;
                    break;
                case KEY_LEFT :
                    position--;
                    break;
                case 127 :
                    // TODO: clearing old chars
                    if (position != 0){
                        contents.erase(position - 1, 1);
                        position--;
                    }
                    break;
                default:
                    contents.insert(position, 1, (char) c);
                    position++;

            }
        }
    };

// used to provide button options to the user
// selecting a button can close the dialog if the exitOnSelect flag is true
// otherwise, selected button(s) have a distinct highlight
    class ButtonsElement : public DialogElement {
    public:
        std::vector<std::string> options;
        std::vector<std::string>::iterator selected;
        short numOptions;
        bool eos;
        // TODO: this
        short intendedYLevel = 3;

        ButtonsElement(std::vector<std::string> opts, bool exitOnSelect){
            options = opts;
            numOptions = opts.size();
            eos = exitOnSelect;
            selected = options.begin();
        }

        void onFocus(){
            isFocused = true;
            curs_set(0);
            return;
        }

        void deFocus(){
            isFocused = false;
            curs_set(1);
            return;
        }

        int requestNumLines(int width){
            // TODO: make this accurate
            return 3;
        }

        void refresh(WINDOW* win, int startY){
            // TODO: center buttons
            intendedYLevel = startY + 1;
            int currentX = 0;
            for(std::string s: options){
                if (isFocused & (s == *selected)){
                    draw_box(win, COLOR_PAIR(borderFocusedColor), intendedYLevel - 1, currentX, intendedYLevel + 1, currentX + s.size() + 1);
                } else {
                    draw_box(win, COLOR_PAIR(borderUnfocusedColor), intendedYLevel - 1, currentX, intendedYLevel + 1, currentX + s.size() + 1);
                }
                mvwaddstr(win, intendedYLevel, currentX + 1, s.data());
                currentX += s.size() + 2;
            }
            return;
        }

        void placeCursor(WINDOW* win, int startY){
            return;
        }

        void handleInput(int c){
           // logMessage("ButtonDialog received input: " + std::to_string(c));
            switch(c){
                case KEY_RIGHT:
                  //  logMessage(*selected);
                    selected++;
                    if (selected == options.end()){
                        selected = options.begin();
                    }
                    break;
                case KEY_LEFT:
                 //   logMessage("left");
                    if (selected == options.begin()){
                        selected = options.end() - 1;
                    } else {
                        selected--;
                    }
                    break;
                case KEY_ENTER:
                    if(eos){
                        // TODO: returning values to where they should go
                        return;
                    }
                    else {
                        return;
                    }
                    break;
                default:
                    break;
            }
            return;
        }

    };

// used to display a string to the user
    class StringElement : public DialogElement {
    public:
        const char* message;
        int length;

        StringElement(const char* str, int len){ // see https://www.oreilly.com/library/view/optimized-c/9781491922057/ch04.html for argument
            message = str;
            length = len;
        }
        ~StringElement(){
            delete message;
        }

        void onFocus(){
            isFocused = true;
            curs_set(0);
            return;
        }

        void deFocus(){
            isFocused = false;
            curs_set(1);
            return;
        }

        int requestNumLines(int width){
            // round up division. this prolly sucks tbh
            return length / width + (length % width != 0);
        }

        void refresh(WINDOW* win, int yval){
            mvwaddstr(win, yval, 0, message);
        }

        void placeCursor(WINDOW* win, int startY){
            return;
        }

        void handleInput(int c){
            // do nothing
            return;
        }

    };

    class Dialog : public window::Window{
    protected:
        std::vector<std::shared_ptr<DialogElement>> elements;
        std::vector<std::shared_ptr<DialogElement>>::iterator currentElement;
        // note that this is not guaranteed to be accurate, but it was updated the last time the Dialog was resized
        // if it is empty on a refresh, it is initialized
        std::vector<int> cached_startYs_cumulative;

    public:

        Dialog(std::vector<std::shared_ptr<DialogElement>> els){
            getmaxyx(stdscr, screen_rows, screen_cols);

            // assuming that we want the max width to be 1/3 of the screen (idk i just picked this #)
            int width = (screen_cols + 2)/3; // this looks jank but stack overflow says this will round the division up

            elements = els;

            // this is hacky, unexpected, and does nothing right now
            elements.push_back(std::make_shared<ButtonsElement>(std::vector<std::string> {"exit"}, false));

            int lines = 0;
            for(std::vector<std::shared_ptr<DialogElement>>::iterator it = elements.begin(); it != elements.end(); it++){
                lines += (*it)->requestNumLines(width);
            }
            height = lines;

            // center dialog box
            int start_x = (screen_cols + 1)/2 - (width + 1)/2; // again, rounding up the division
            int start_y = (screen_rows + 1)/2 - (height + 1)/2;

            // offsets to account for the border
            Window::create_windows(height + 2, width + 2, start_y - 1, start_x - 1, height, width, start_y, start_x);
            currentElement = elements.begin();

            (*currentElement)->onFocus();

            refresh();
            wrefresh(Window::win);
        }

        void reloadHeights(){
            int currY = 0;
            cached_startYs_cumulative.clear();
            for(int i = 0; i < elements.size(); i++){
                cached_startYs_cumulative.push_back(currY);
                currY += elements[i]->requestNumLines(width);
            }
        }

        void refresh(){
            if(cached_startYs_cumulative.empty()){
                reloadHeights();
            }
            for(int i = 0; i < elements.size(); i++){
                wattron(win, COLOR_PAIR(textColor));
                elements[i]->refresh(Window::win, cached_startYs_cumulative[i]);
                wattroff(win, COLOR_PAIR(textColor));
            }
            // note: it - vector.begin() gets the index of iterator it
            (*currentElement)->placeCursor(win, cached_startYs_cumulative[currentElement - elements.begin()]);
            wrefresh(Window::win);
        }

        void handleInput(int c){
            switch(c){
                case('\t') :
                case(KEY_DOWN) :
                    (*currentElement)->deFocus();
                    currentElement++;
                    if (currentElement == elements.end()){
                        currentElement = elements.begin();
                    }
                    (*currentElement)->onFocus();
                    break;

                case(KEY_BTAB) :
                case(KEY_UP) :
                    (*currentElement)->deFocus();
                    if (currentElement == elements.begin()){
                        currentElement = elements.end();
                    }
                    currentElement--;
                    (*currentElement)->onFocus();
                    break;

                default:
                    (*currentElement)->handleInput(c);
                    break;
            };
            refresh();
        }

        // TODO: these
        void resize(int, int){
            return;
        }

        void onFocus(){
            (*currentElement)->onFocus();
            return;
        }

        void deFocus(){
            (*currentElement)->deFocus();
            return;
        }
    };

}