#pragma once

#include <gtkmm.h>
#include "settings/pathscolumns.hpp"
#include "librarycontroller.hpp"
#include "benchmark.hpp"
#include <filesystem>

class SettingsWindow : public Gtk::Window {
public:
    PathsColumns mModelColumns;

    SettingsWindow(LibraryController* controller);
    ~SettingsWindow();

    void updatePaths(std::list<std::filesystem::path> paths);

    void updateProgress(bool status, double progress, std::string stats);

protected:
    Gtk::FileChooserButton mFileChooserButton;
    Gtk::Button mAddPathButton;
    Gtk::TreeView* mPathsView;
    Gtk::Button mImportButton;
    Gtk::Button mCleanImportButton;
    Gtk::Button mRemovePathButton;
    Gtk::Statusbar mStatusbar;
    Gtk::ProgressBar mProgressBar;
    Gtk::Label mProgressLabel;
    Gtk::Label mStatusLabel;
    Glib::RefPtr<Gtk::ListStore> mListStore;
    LibraryController* mController;
    bool mStatus = false;

    Benchmark mBenchmark;

    void on_addPath_button_click();
    void on_removePath_button_click();
    void on_import_button_click();
    void on_clean_import_button_click();

    void on_benchmark_button_click();
};