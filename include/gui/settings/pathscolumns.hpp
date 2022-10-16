#pragma once

#include <gtkmm.h>

class PathsColumns : public Gtk::TreeModel::ColumnRecord {
public:
    PathsColumns();
    Gtk::TreeModelColumn<std::string> mColumnPath;
};