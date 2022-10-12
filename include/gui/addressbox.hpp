#pragma once

#include <gtkmm.h>

#include "treefilebrowser.hpp"
#include "filebrowserfilter.hpp"
#include "medialibrary.hpp"
#include "librarycontroller.hpp"

class Addressbox : public Gtk::HBox
{
public:
    void setTreeFilebrowser(TreeFilebrowser* newTreeFilebrowser);
    void setAddress(std::string newAddres);
    void initialize(MediaLibrary* mediaLibrary, LibraryController* libraryController);
    std::string getAddress();

    /**
     * Makes path valid (removes trailing /)
     * 
     * @param path String we need to correct.
     * 
     * @return New corrected string.
     */
    std::string makeValidPath(std::string path);

    void updateProgress(bool status, float progress, std::string stats);
    
    void on_go_button_click();

    Addressbox();
    ~Addressbox();
private:
    Gtk::Button mGoButton;
    Gtk::Label mProgressLabel;
    Gtk::Entry mAddressBar;
    std::string mAddress;
    Gtk::IconView* mIconView;
    MediaLibrary* mMediaLibrary;
    LibraryController* mLibraryController;
    bool mStatus = false;
};