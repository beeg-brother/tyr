//
// Created by derek on 6/1/20.
// For documentation, see editor.cpp
//

#ifndef TYR_EDITOR_H
#define TYR_EDITOR_H

#include "constants.h"
#include "cursor.h"
#include "window.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>


namespace editor {

    class Editor : public window::Window{
    public:

        std::vector<std::string> strs;

        Cursor cursor;

        int scroll_offset;
        int window_width;
        int window_height;

        std::filesystem::path filepath;

        Editor(int h, int w, int y0, int x0);

        Editor(int h, int w, int y0, int x0, std::filesystem::path filename);

        void resize(int, int);
        void onFocus();
        void deFocus();
        void handleInput(int c);

        WINDOW* getEditorWindow();
        PANEL* getEditorPanel();

        void create_windows(int h, int w, int y0, int x0);
        void rewrite_line_nums();
        void rewrite();
    };
}

#endif //TYR_EDITOR_H
