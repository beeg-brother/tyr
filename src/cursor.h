//
// Created by derek on 6/1/20.
//

#ifndef TYR_CURSOR_H
#define TYR_CURSOR_H

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

#endif //TYR_CURSOR_H
