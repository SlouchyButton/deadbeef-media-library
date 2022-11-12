#pragma once

#include <filesystem>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <chrono>

#include "medialibrary.hpp"
#include "modelcolumns.hpp"


class LibraryController {
public:
    LibraryController();
    ~LibraryController();

    void initialize(Glib::RefPtr<Gtk::ListStore> listStorePtr);

    void clearLibrary();

    void startImport();
    void stopImport();

    void startMaintenanceThread();
    void stopMaintenanceThread();

    double getImportProgress() const;
    bool getImportStatus() const;
    double getMaintenanceProgress() const;
    bool getMaintenanceStatus() const;

    void addCallback(std::function<void()> callback);

    void refreshModel();

    MediaLibrary* getMediaLibrary();
    MediaLibrary* mMediaLibrary;
private:
    ModelColumns mModelColumns;
    Glib::RefPtr<Gtk::ListStore> mListStore;
    std::filesystem::path mLibraryPath;
    std::map<std::filesystem::path, int> mFoundPaths;

    std::thread* mImportLibraryThread;
    std::thread* mMaintenanceThread;
    std::thread* mInotifyThread;

    std::atomic<bool> mImportThreadRun = true;
    std::atomic<bool> mMaintenanceThreadRun = true;

    std::atomic<double> mImportProgress = 0;
    std::atomic<bool> mImportStatus = false;
    std::atomic<double> mMaintenanceProgress = 0;
    std::atomic<bool> mMaintenanceStatus = false;

    std::atomic<bool> mImportPending = false;
    std::queue<std::filesystem::path> mImportQueue;
    std::queue<std::filesystem::path> mDeleteQueue;
    std::map<std::filesystem::path, std::chrono::time_point<std::chrono::steady_clock>> mRefreshQueue;
    std::mutex mRefreshQueueMutex;


    std::condition_variable mMaintenanceCtrl;
    std::mutex mMaintenanceMutex;

    std::map<std::filesystem::path, int> watchDescriptors;
    std::map<int, std::filesystem::path> watchDescriptorsPath;
    int fd;

    std::vector<std::function<void()>> mCallbacks;

    void importLibraryThread();
    void importFolder(std::filesystem::path path);

    void maintenanceThread();

    void inotifyThread();
    void updateInotifyFolders();

    void notifyCallbacks();
};