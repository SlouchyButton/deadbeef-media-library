#pragma once

#include <gtkmm.h>

#include "treefilebrowser.hpp"
#include "searchbar.hpp"
#include "filebrowserfilter.hpp"
#include "medialibrary.hpp"
#include "librarycontroller.hpp"
#include "settings/window.hpp"

class Addressbox : public Gtk::HBox
{
public:
    void setTreeFilebrowser(TreeFilebrowser* newTreeFilebrowser);
    void initialize(MediaLibrary* mediaLibrary, LibraryController* libraryController, SettingsWindow* settingsWindow, FilebrowserFilter* filter);

    void updateProgress(bool status, double progress, std::string stats);
    
    void on_go_button_click();
    void on_settings_button_click();

    Addressbox();
    ~Addressbox();
private:
    Gtk::Button mRefreshButton;
    Gtk::Button mSettingsButton;
    Gtk::Label mProgressLabel;
    std::string mAddress;
    Gtk::IconView* mIconView;
    Searchbar mSearchbar;
    MediaLibrary* mMediaLibrary;
    LibraryController* mLibraryController;
    SettingsWindow* mSettingsWindow;
    FilebrowserFilter* mFilebrowserFilter;
    bool mStatus = false;
};