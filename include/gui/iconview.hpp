#pragma once

#include <gtkmm.h>

#include "modelcolumns.hpp"

class IconView : public Gtk::IconView {
public:
    IconView();
    ~IconView();

    void initialize();
private:
    std::map<std::string, Gtk::CellRendererText> mTextRenderers;
    ModelColumns mModelColumns;

};