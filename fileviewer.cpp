//
// Created by derek on 6/1/20.
//

#include "constants.h"
#include "filemenu.cpp"
#include "fileviewer.h"
#include "window.h"
#include <map>


namespace fileviewer {
    FileViewer::FileViewer(int h, int w, int y0, int x0, std::map<std::string, int> color_map) {
        create_windows(h, w, y0, x0);
        fsys::path cwd = fsys::current_path();
        nmenu->setWindow(getWindow());
        nmenu->setMenuItems(nmenu->getDirFiles(cwd));
        nmenu->setColorMap(color_map);
        nmenu->drawMenu();

    }

    // my new version  of menus
    filemenu::Menu *nmenu = new filemenu::Menu();

    // TODO: resizing
    void FileViewer::resize(int, int) {
        return;
    };

    void FileViewer::create_windows(int h, int w, int y0, int x0) {
        Window::create_windows(h, w, y0, x0, h - 2, w - 2, y0 + 1, x0 + 1);
    }

    WINDOW *FileViewer::getWindow() {
        return Window::win;
    }

    void FileViewer::onFocus() {
        return;
    }

    void FileViewer::deFocus() {
        return;
    }

    void FileViewer::handleInput(int c) {
        switch (c) {
            case KEY_DOWN:
                nmenu->menu_down();
                break;
            case KEY_UP:
                nmenu->menu_up();
                break;
            case KEY_RIGHT:
                nmenu->menu_right();
                break;
            case KEY_LEFT:
                nmenu->menu_left();
                break;
        }
    }
}