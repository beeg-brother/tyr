//
// Created by derek on 6/1/20.
// For documentation, see fileviewer.cpp
//

#ifndef TYR_FILEVIEWER_H
#define TYR_FILEVIEWER_H

#include "constants.h"
#include "filemenu.h"
#include "window.h"
#include <map>

namespace fileviewer {
    class FileViewer : public window::Window {
    public:
        FileViewer(int h, int w, int y0, int x0, std::map<std::string, int> color_map);

        filemenu::Menu *nmenu = new filemenu::Menu();
        void resize(int, int);

        void create_windows(int h, int w, int y0, int x0);
        WINDOW *getWindow();
        void onFocus();
        void deFocus();
        void handleInput(int c);
    };
}

#endif //TYR_FILEVIEWER_H
