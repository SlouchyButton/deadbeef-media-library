#pragma once

#include <gtkmm.h>

#include "treefilebrowser.hpp"
#include "searchbar.hpp"
#include "addressbox.hpp"
#include "filebrowserfilter.hpp"
#include "treepopup.hpp"
#include "medialibrary.hpp"
#include "modelcolumns.hpp"
#include "librarycontroller.hpp"
#include "iconview.hpp"

class Container : public Gtk::VBox {
public:
    ModelColumns mModelColumns;

    Container();
    ~Container();

    void initialize();

    void notify();
private:
    Searchbar mSearchbar;
    MediaLibrary mMediaLibrary;
    LibraryController mLibraryController;
    IconView mIconView;
    Glib::RefPtr<Gtk::ListStore> mListStore;
    TreePopup mTreePopup;
    Addressbox mAddressbox;
    Gtk::ScrolledWindow mScrolledWindow;
    Glib::RefPtr<TreeFilebrowser> mTreeFilebrowser;
    Glib::RefPtr<FilebrowserFilter> mFilebrowserFilter;
    Glib::Dispatcher mDispatcher;

    void buildTreeview();

    void onNotify();
};
