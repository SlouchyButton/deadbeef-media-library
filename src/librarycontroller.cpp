#include "librarycontroller.hpp"

#include "filebrowser.hpp"
#include "utils.hpp"
#include "plugin.hpp"

#include <fstream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <iostream>

LibraryController::LibraryController() {
}

LibraryController::~LibraryController() {
}

void LibraryController::initialize(MediaLibrary* mediaLibraryPtr, Glib::RefPtr<Gtk::ListStore> listStorePtr) {
    this->mMediaLibrary = mediaLibraryPtr;
    this->mListStore = listStorePtr;

    std::string dbLibraryPath = deadbeef->get_system_dir(DDB_SYS_DIR_CACHE);
    this->mLibraryPath = dbLibraryPath + "/media-library/library.bin";

    pluginLog(2, "Library Controller - Starting maintenance thread");
    this->mMaintenanceThread = new std::thread(&LibraryController::maintenanceThread, this);
}

void LibraryController::startImport() {
    this->mImportLibraryThread = new std::thread(&LibraryController::importLibraryThread, this);
}

void LibraryController::addCallback(std::function<void()> callback) {
    this->mCallbacks.push_back(callback);
}

void LibraryController::importLibraryThread() {
    pluginLog(2, "Library Controller - Importing library");
    this->mImportStatus = true;
    this->notifyCallbacks();
    
    pluginLog(2, "Import Thread - Finding all files in paths");
    int countTotal = 0;
    std::vector<std::filesystem::directory_entry> directories = std::vector<std::filesystem::directory_entry>();
    for (auto path : this->mMediaLibrary->getSearchPaths()) {
        pluginLog(2, "Import Thread - Searching path: " + path.string());
        std::vector<std::filesystem::directory_entry> filelist = Filebrowser::getFileList(path, true, false);
        int count = filelist.size();
        if (count > 0) {
            for (auto &entry : filelist) {
                directories.push_back(entry);
            }
            countTotal += count;
        }
    }
    pluginLog(2, "Import Thread - Found " + std::to_string(countTotal) + " items");


    int i = 0;
    for (auto &entry : directories) {
        if (!mImportThreadRun.load()) {
            pluginLog(2, "Import Thread - Import cancelled by user");
            this->mImportProgress = 0;
            this->mImportStatus = false;
            this->notifyCallbacks();
            return;
        }

        this->mImportProgress = (float)i / (float)countTotal;
        this->notifyCallbacks();
        
        if (entry.is_directory() && !std::filesystem::is_empty(entry)) {
            this->importFolder(entry.path());
        } else {
            this->mMediaLibrary->addMediaFile(entry.path());
        }

        i++;
    }

    pluginLog(2, "Import Thread - Library imported");

    pluginLog(2, "Import Thread - Refreshing model");
    this->refreshModel();

    //Everything done, cleaning up and notifying for the last time
    this->mImportStatus = false;
    this->mImportProgress = 0;
    this->notifyCallbacks();

    //Move this to a maintenance thread
    pluginLog(2, "Import Thread - Saving library to file");
    if (!std::filesystem::exists(this->mLibraryPath.parent_path())) {
        std::filesystem::create_directories(this->mLibraryPath.parent_path());
    }
    try {
        std::ofstream ofs(this->mLibraryPath, std::ios::binary);
        boost::archive::binary_oarchive oa(ofs);
        oa << mMediaLibrary;
        pluginLog(2, "Import Thread - Library saved");
    } catch (const boost::archive::archive_exception &e) {
        std::string err = e.what();
        pluginLog(0, "Import Thread - Couldn't save library to file " + this->mLibraryPath.string() + " with error: " + err);
    }
}

void LibraryController::importFolder(std::filesystem::path path) {
    auto filelist = Filebrowser::getFileList(path, false, false);
    int count = filelist.size();
    if (count > 0) {
        for (auto &entry : filelist) {
            if (!mImportThreadRun.load()) {
                pluginLog(2, "Import Thread - Stopping folder import, requested by user");
                return;
            }

            if (entry.is_directory() && !std::filesystem::is_empty(entry)) {
                this->importFolder(entry.path());
            } else {
                this->mMediaLibrary->addMediaFile(entry.path());
                this->notifyCallbacks();
            }
        }
    }
}

void LibraryController::maintenanceThread() {
    if (std::filesystem::exists(this->mLibraryPath)) {
        this->mMaintenanceStatus = true;
        pluginLog(2, "Maintenance Thread - Found saved library, loading");
        try {
            std::ifstream ifs(this->mLibraryPath, std::ios::binary);
            boost::archive::binary_iarchive ia(ifs);
            ia >> this->mMediaLibrary;
            pluginLog(2, "Maintenance Thread - Library loaded, loading covers");
            this->mMediaLibrary->loadCovers();
            pluginLog(2, "Maintenance Thread - Covers loaded, library ready");
            this->refreshModel();
        } catch (const boost::archive::archive_exception &e) {
            std::string err = e.what();
            pluginLog(0, "Maintenance Thread - Couldn't load library from file " + this->mLibraryPath.string() + " with error: " + err);
        }
        this->mMaintenanceStatus = false;
        this->notifyCallbacks();
    }
    pluginLog(2, "Maintenance Thread - Maintenence thread not fully implemented yet. No more work left, exiting");

    //Get from OpenRGB, so it can refresh in period of long time, but be instantly stoppable
    /*while (true) {
        if (this->mMediaLibrary->isDirty()) {
            pluginLog(2, "Library is dirty, saving");
            std::ofstream ofs(this->mLibraryPath, std::ios::binary);
            boost::archive::binary_oarchive oa(ofs);
            oa << this->mMediaLibrary;
            pluginLog(2, "Library saved");
        }
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }*/
}

void LibraryController::notifyCallbacks() {
    int i = 0;
    for (auto &callback : this->mCallbacks) {
        callback();
        i++;
    }
    //pluginLog(2, "Library Controller - Notified " + std::to_string(i) + " registered callbacks (dispatchers)");
}

void LibraryController::refreshModel() {
    this->mListStore->clear();
    Gtk::TreeModel::iterator iter;
    for (Album* &entry : this->mMediaLibrary->getAlbums()) {
        iter = this->mListStore->append();
        Gtk::TreeModel::Row row = *iter;
        row[mModelColumns.mColumnIcon] = entry->Cover->CoverPixbuf;
        row[mModelColumns.mColumnTitle] = entry->Name;
        row[mModelColumns.mColumnSubtitle] = entry->Artist;
        row[mModelColumns.mColumnAlbumPointer] = entry;
        row[mModelColumns.mColumnVisibility] = true;
    }
}

void LibraryController::stopImport() {
    if(this->mImportLibraryThread != NULL) {
        this->mImportThreadRun = false;
        this->mImportLibraryThread->join();
        this->mImportLibraryThread = NULL;
        this->mImportThreadRun = true;
    }
}

void LibraryController::stopMaintenanceThread() {
    if(this->mMaintenanceThread != NULL) {
        this->mMaintenanceThreadRun = false;
        this->mMaintenanceThread->join();
        this->mMaintenanceThread = NULL;
        this->mMaintenanceThreadRun = true;
    }
}

float LibraryController::getImportProgress() const {
    return this->mImportProgress;
}
bool LibraryController::getImportStatus() const {
    return this->mImportStatus;
}
float LibraryController::getMaintenanceProgress() const {
    return this->mMaintenanceProgress;
}
bool LibraryController::getMaintenanceStatus() const {
    return this->mMaintenanceStatus;
}