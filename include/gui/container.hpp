#pragma once

#include <gtkmm.h>

#include "treefilebrowser.hpp"
#include "addressbox.hpp"
#include "filebrowserfilter.hpp"
#include "treepopup.hpp"
#include "medialibrary.hpp"
#include "modelcolumns.hpp"
#include "librarycontroller.hpp"
#include "iconview.hpp"
#include "settings/window.hpp"

class Container : public Gtk::VBox {
public:
    ModelColumns mModelColumns;

    Container();
    ~Container();

    void initialize();

    void notify();
private:
    LibraryController mLibraryController;
    Addressbox mAddressbox;
    Gtk::ScrolledWindow mScrolledWindow;
    IconView mIconView;
    Glib::RefPtr<Gtk::ListStore> mListStore;
    TreePopup mTreePopup;
    Glib::RefPtr<TreeFilebrowser> mTreeFilebrowser;
    Glib::RefPtr<FilebrowserFilter> mFilebrowserFilter;
    Glib::Dispatcher mDispatcher;
    SettingsWindow* mSettingsWindow;

    void buildTreeview();

    void onNotify();
};
