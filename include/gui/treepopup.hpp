#pragma once

#include <gtkmm.h>

#include "treefilebrowser.hpp"
#include "filebrowserfilter.hpp"
#include "addressbox.hpp"
#include "medialibrary.hpp"
#include "modelcolumns.hpp"
#include "librarycontroller.hpp"

class TreePopup : public Gtk::Menu {
public:
    /**
     * Binds events to be callable etc.
     */
    void initialize(Gtk::IconView* iconView, Glib::RefPtr<FilebrowserFilter> model, LibraryController* libraryController, Addressbox* addressbox /*TreeFilebrowser* treefb, FilebrowserFilter* filter*/);

    Gtk::TreeModel::Path getCurrentPath() const;

    TreePopup();
    ~TreePopup();
private:
    /**
     * Function arguments for addToPlaylist when forking thread
     */
    struct structAddToPlaylist {
        std::vector<Album*> albums;
        bool replace;
    };

    structAddToPlaylist mDataHolder;

    Glib::RefPtr<Gio::Menu> mMenu;
    Glib::RefPtr<Gio::SimpleActionGroup> mActionGroup;
    Gtk::IconView* mIconView;
    Glib::RefPtr<FilebrowserFilter> mModel;
    FilebrowserFilter* mFilter;
    Addressbox* mAddressbox;
    LibraryController* mLibraryController;
    ModelColumns mModelColumns;
    Gtk::TreeModel::Path mPath = Gtk::TreeModel::Path();

    bool on_click(GdkEventButton* event);

    // Various events for the popup
    void popup_add();
    void popup_replace();
    void popup_open_folder();
    void popup_enter_directory();
    void popup_go_up();
    void popup_refresh();
    void popup_metadata();

    /**
     * Adds to playlist files or folders.
     * 
     * @param uris URIs of selected rows.
     * @param address Every URI has to be constructed first, pass mAddressbox->getAddress().
     * @param replace Whether current playlist should be replaced.
     */
    static void addToPlaylist(std::vector<Album*> albums, bool replace);

    /**
     * Calls addToPlaylist, make sure to pass structAddToPlaylist as argument.
     */
    static void threadedAddToPlaylist(void* ctx);

    /**
     * Calls addToPlaylist, make sure to pass structAddToPlaylist as argument.
     */
    static void threadedReplacePlaylist(void* ctx);

    Album* getAlbumFromPath(Gtk::TreeModel::Path path);

};