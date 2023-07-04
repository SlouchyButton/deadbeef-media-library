#pragma once

#include <gtkmm.h>
#include "settings/pathscolumns.hpp"
#include "librarycontroller.hpp"
#include <filesystem>

class SettingsWindow : public Gtk::Window {
public:
    PathsColumns mModelColumns;

    SettingsWindow(LibraryController* controller);
    ~SettingsWindow();

    void updatePaths();

    void updateProgress(bool status, double progress, std::string stats);

protected:
    Gtk::FileChooserButton mFileChooserButton;
    Gtk::Button mAddPathButton;
    Gtk::Button mAddIgnoredPathButton;
    Gtk::TreeView* mPathsView;
    Gtk::TreeView* mIgnoredPathsView;
    Gtk::Button mImportButton;
    Gtk::Button mCleanImportButton;
    Gtk::Button mRemovePathButton;
    Gtk::Button mRemoveIgnoredPathButton;
    Gtk::Statusbar mStatusbar;
    Gtk::ProgressBar mProgressBar;
    Gtk::Label mProgressLabel;
    Gtk::Label mStatusLabel;
    Glib::RefPtr<Gtk::ListStore> mPathStore;
    Glib::RefPtr<Gtk::ListStore> mIgnoredPathStore;
    LibraryController* mController;
    bool mStatus = false;

    void on_addPath_button_click();
    void on_addIgnoredPath_button_click();
    void on_removePath_button_click();
    void on_removeIgnoredPath_button_click();
    void on_import_button_click();
    void on_clean_import_button_click();
};