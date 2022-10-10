#pragma once

#include <gtkmm.h>
#include "album.hpp"

class ModelColumns : public Gtk::TreeModel::ColumnRecord {
public:
    ModelColumns();
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> mColumnIcon;
    Gtk::TreeModelColumn<Album*> mColumnAlbumPointer;
    Gtk::TreeModelColumn<std::string> mColumnTitle;
    Gtk::TreeModelColumn<std::string> mColumnSubtitle;
    Gtk::TreeModelColumn<bool> mColumnVisibility;
};