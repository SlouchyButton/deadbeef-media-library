#pragma once

#include <gtkmm.h>

#include "treefilebrowser.hpp"
#include "modelcolumns.hpp"

class FilebrowserFilter : public Gtk::TreeModelFilter
{
public:
    FilebrowserFilter(Glib::RefPtr<Gtk::TreeModel> model);
    ~FilebrowserFilter();
    static Glib::RefPtr<FilebrowserFilter> create(Glib::RefPtr<Gtk::ListStore> treefb);
    void setNeedle(std::string newNeedle);
    void setModel(TreeFilebrowser* treefb);
private:
    std::string mNeedle;
    TreeFilebrowser* mTreeFilebrowser;
    ModelColumns mModelColumns;


    bool filter_func(Gtk::TreeIter iter);
};