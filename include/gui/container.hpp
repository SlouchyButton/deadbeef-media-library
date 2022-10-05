#pragma once

#include <gtkmm.h>

#include "treefilebrowser.hpp"
#include "searchbar.hpp"
#include "addressbox.hpp"
#include "filebrowserfilter.hpp"
#include "treepopup.hpp"
#include "medialibrary.hpp"

class Container : public Gtk::VBox {
public:
    Container();
    ~Container();
    void initialize();
private:
    Searchbar mSearchbar;
    Addressbox mAddressbox;
    MediaLibrary mMediaLibrary;
    Gtk::IconView mIconView;
    TreePopup mTreePopup;
    Gtk::ScrolledWindow mScrolledWindow;
    Glib::RefPtr<TreeFilebrowser> mTreeFilebrowser;
    Glib::RefPtr<FilebrowserFilter> mFilebrowserFilter;
    std::map<std::string, Gtk::CellRendererText> mTextRenderers;

    void buildTreeview();
};
