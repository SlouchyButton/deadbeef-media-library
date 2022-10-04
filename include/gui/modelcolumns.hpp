#pragma once

#include <gtkmm.h>
#include "album.hpp"

class ModelColumns : public Gtk::TreeModel::ColumnRecord {
public:
    ModelColumns();
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> mColumnIcon;
    Gtk::TreeModelColumn<std::string> mColumnName;
    Gtk::TreeModelColumn<Album*> mColumnAlbumPointer;
    Gtk::TreeModelColumn<std::string> mColumnTooltip;
    Gtk::TreeModelColumn<bool> mColumnVisibility;
};