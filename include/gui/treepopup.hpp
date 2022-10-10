#pragma once

#include <gtkmm.h>

#include "treefilebrowser.hpp"
#include "filebrowserfilter.hpp"
#include "addressbox.hpp"
#include "medialibrary.hpp"
#include "modelcolumns.hpp"

class TreePopup : public Gtk::Menu {
public:
    /**
     * Binds events to be callable etc.
     */
    void initialize(Gtk::IconView* iconView, Glib::RefPtr<Gtk::ListStore> model, Addressbox* addressbox /*TreeFilebrowser* treefb, FilebrowserFilter* filter*/);
    TreePopup();
    ~TreePopup();
private:
    /**
     * Function arguments for addToPlaylist when forking thread
     */
    struct structAddToPlaylist {
        std::vector<Album*> albums;
        std::string address;
        bool replace;
    };

    structAddToPlaylist mDataHolder;

    Glib::RefPtr<Gio::Menu> mMenu;
    Glib::RefPtr<Gio::SimpleActionGroup> mActionGroup;
    Gtk::IconView* mIconView;
    Glib::RefPtr<Gtk::ListStore> mModel;
    FilebrowserFilter* mFilter;
    Addressbox* mAddressbox;
    ModelColumns mModelColumns;

    void on_click(const Gtk::TreeModel::Path& path);

    // Various events for the popup
    void popup_add();
    void popup_replace();
    void popup_open_folder();
    void popup_enter_directory();
    void popup_go_up();
    void popup_refresh();

    /**
     * Adds to playlist files or folders.
     * 
     * @param uris URIs of selected rows.
     * @param address Every URI has to be constructed first, pass mAddressbox->getAddress().
     * @param replace Whether current playlist should be replaced.
     */
    static void addToPlaylist(std::vector<Album*> albums, std::string address, bool replace);

    /**
     * Calls addToPlaylist, make sure to pass structAddToPlaylist as argument.
     */
    static void threadedAddToPlaylist(void* ctx);

    /**
     * Calls addToPlaylist, make sure to pass structAddToPlaylist as argument.
     */
    static void threadedReplacePlaylist(void* ctx);

    // Helper functions
    /**
     * Returns selected URI from mTreeView.
     * 
     * @return URI of selected row
     */
    std::string getSelectedURI();

    /**
     * Returns all selected URIs from mTreeView.
     * 
     * @return Vector of selected rows
     */
    std::vector<std::string> getSelectedURIs();

    /**
     * Returns true if any row in mTreeView is selected.
     * 
     * @return True if any row is selected, false otherwise.
     */
    bool hasSelected();
};