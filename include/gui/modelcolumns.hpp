#pragma once

#include <gtkmm.h>
#include "album.hpp"

class ModelColumns : public Gtk::TreeModel::ColumnRecord {
public:
    ModelColumns();
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> mColumnIcon;
    Gtk::TreeModelColumn<std::string> mColumnTooltip;
    Gtk::TreeModelColumn<Album*> mColumnAlbumPointer;
    Gtk::TreeModelColumn<std::string> mColumnTitle;
    Gtk::TreeModelColumn<std::string> mColumnSubtitle;
    Gtk::TreeModelColumn<std::string> mColumnCustom1;
    Gtk::TreeModelColumn<std::string> mColumnCustom2;
    Gtk::TreeModelColumn<std::string> mColumnCustom3;
    Gtk::TreeModelColumn<std::string> mColumnCustom4;
    Gtk::TreeModelColumn<bool> mColumnVisibility;
};