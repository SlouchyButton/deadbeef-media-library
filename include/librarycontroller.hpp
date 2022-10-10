#pragma once

#include <filesystem>
#include <thread>
#include <atomic>
#include <memory>

#include "medialibrary.hpp"
#include "modelcolumns.hpp"

class LibraryController {
public:
    LibraryController();
    ~LibraryController();

    void initialize(MediaLibrary* mediaLibraryPtr, Glib::RefPtr<Gtk::ListStore> listStorePtr);

    void startImport();
    void stopImport();

    void startMaintenanceThread();
    void stopMaintenanceThread();

    float getImportProgress() const;
    bool getImportStatus() const;
    float getMaintenanceProgress() const;
    bool getMaintenanceStatus() const;

    void addCallback(std::function<void()> callback);
private:
    MediaLibrary* mMediaLibrary;
    ModelColumns mModelColumns;
    Glib::RefPtr<Gtk::ListStore> mListStore;
    std::filesystem::path mLibraryPath;

    std::thread* mImportLibraryThread;
    std::thread* mMaintenanceThread;

    std::atomic<bool> mImportThreadRun = true;
    std::atomic<bool> mMaintenanceThreadRun = true;

    std::atomic<float> mImportProgress = 0;
    std::atomic<bool> mImportStatus = false;
    std::atomic<float> mMaintenanceProgress = 0;
    std::atomic<bool> mMaintenanceStatus = false;

    std::vector<std::function<void()>> mCallbacks;

    void importLibraryThread();
    void maintenanceThread();

    void notifyCallbacks();

    void importFolder(std::filesystem::path path);

    void refreshModel();
};