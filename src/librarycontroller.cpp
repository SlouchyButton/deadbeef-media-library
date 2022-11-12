#include "librarycontroller.hpp"

#include "filebrowser.hpp"
#include "utils.hpp"
#include "plugin.hpp"

#include <fstream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <iostream>

#include <sys/inotify.h>
#include <sys/types.h>
#include <errno.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

//TODO: This file is starting to be a chonky boi, maybe split it up into multiple files?
//Proposal could split each thread into own file, and let this
//control the whole thing, as the name suggests anyway
//TODO: With previous proposal, inotify should be own thing anyway, because it will have
//to be implemented differently on other platforms

LibraryController::LibraryController() {
    this->mMediaLibrary = new MediaLibrary();
}

LibraryController::~LibraryController() {
    this->stopImport();
    this->stopMaintenanceThread();
}

void LibraryController::initialize(Glib::RefPtr<Gtk::ListStore> listStorePtr) {
    this->mListStore = listStorePtr;

    std::string dbLibraryPath = deadbeef->get_system_dir(DDB_SYS_DIR_CACHE);
    dbLibraryPath += "/media-library";
    
    if (!std::filesystem::exists(dbLibraryPath)) {
        std::filesystem::create_directories(dbLibraryPath);
    }

    this->mLibraryPath = dbLibraryPath + "/library_1_1.bin";

    pluginLog(2, "Library Controller - Starting maintenance thread");
    this->mMaintenanceThread = new std::thread(&LibraryController::maintenanceThread, this);
}

void LibraryController::clearLibrary() {
    this->mMediaLibrary->clearLibrary();
    //this->mListStore->clear();
}

void LibraryController::startImport() {
    this->mImportLibraryThread = new std::thread(&LibraryController::importLibraryThread, this);
}

void LibraryController::addCallback(std::function<void()> callback) {
    this->mCallbacks.push_back(callback);
}

MediaLibrary* LibraryController::getMediaLibrary() {
    return this->mMediaLibrary;
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

    this->mFoundPaths.clear();

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
            this->mMediaLibrary->refreshMediaFile(entry.path());
            this->mFoundPaths[entry.path()] = 1;
        }

        i++;
    }

    pluginLog(2, "Import Thread - Cleaning up non-existing files");
    for (MediaFile* entry : this->mMediaLibrary->getMediaFiles()) {
        if (this->mFoundPaths.find(entry->Path) == this->mFoundPaths.end()) {
            this->mMediaLibrary->removeMediaFile(entry);
        }
    }

    pluginLog(2, "Import Thread - Sorting MediaFiles");
    this->mMediaLibrary->sortMediaFiles();

    pluginLog(2, "Import Thread - Library imported");

    //Everything done, cleaning up and notifying for the last time
    this->mImportStatus = false;
    this->mImportProgress = 0;
    this->notifyCallbacks();
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
                this->mMediaLibrary->refreshMediaFile(entry.path());
                this->mFoundPaths[entry.path()] = 1;
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
    pluginLog(2, "Maintenance Thread - Starting inotify thread");
    this->mInotifyThread = new std::thread(&LibraryController::inotifyThread, this);

    bool skipWait = false;
    while (this->mMaintenanceThreadRun.load()) {
        skipWait = false;
        if (this->mImportPending.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            this->mImportPending = false;
            this->startImport();
        }

        if (!this->mImportQueue.empty() || !this->mDeleteQueue.empty() || this->mRefreshQueue.size() > 0) {
            pluginLog(2, "Maintenance Thread - Queues not empty, starting refresh");
            this->mMaintenanceStatus = true;
            this->notifyCallbacks();
            while (this->mImportQueue.size() > 0) {
                std::filesystem::path path = this->mImportQueue.front();
                pluginLog(2, "Maintenance Thread - Importing " + path.string());
                this->mImportQueue.pop();
                this->mMediaLibrary->refreshMediaFile(path);
            }

            if (this->mRefreshQueue.size() > 0) {
                std::vector<std::filesystem::path> removedPaths = std::vector<std::filesystem::path>();
                this->mRefreshQueueMutex.lock();
                for (auto &entry : this->mRefreshQueue) {
                    if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - entry.second).count() > 3) {
                        pluginLog(2, "Maintenance Thread - Refreshing " + entry.first.string());
                        this->mMediaLibrary->refreshMediaFile(entry.first);
                        removedPaths.push_back(entry.first);
                    } else {
                        pluginLog(2, "Maintenance Thread - Skipping refresh of " + entry.first.string() + " because modification might not be finished");
                        skipWait = true;
                    }
                }
                //Remove all paths that have been refreshed, doing this in previous loop would cause iterator to be invalidated -> segfault
                for (auto &entry : removedPaths) {
                    this->mRefreshQueue.erase(entry);
                }
                this->mRefreshQueueMutex.unlock();
            }

            this->mMediaLibrary->sortMediaFiles();

            while (this->mDeleteQueue.size() > 0) {
                std::filesystem::path path = this->mDeleteQueue.front();
                pluginLog(2, "Maintenance Thread - Deleting " + path.string());
                this->mDeleteQueue.pop();
                this->mMediaLibrary->removeMediaFile(path);
            }
            pluginLog(2, "Maintenance Thread - Refresh done");
            this->mMaintenanceStatus = false;
            this->notifyCallbacks();
        }

        //Libraries can be huge and will most likely reside on a SSD, so we don't want to waste write cycles,
        //if we know that there are ANY updates pending. We save after everythign is done.
        if (this->mMediaLibrary->libraryDirty && !this->mImportStatus && this->mImportQueue.empty() && this->mDeleteQueue.empty() && this->mRefreshQueue.size() == 0) {
            pluginLog(2, "Maintenance Thread - Library is dirty, saving");
            std::ofstream ofs(this->mLibraryPath, std::ios::binary);
            boost::archive::binary_oarchive oa(ofs);
            oa << this->mMediaLibrary;
            this->mMediaLibrary->libraryDirty = false;
            pluginLog(2, "Maintenance Thread - Library saved");
        }

        if (!skipWait) {
            std::unique_lock<std::mutex> l(mMaintenanceMutex);
            mMaintenanceCtrl.wait_for(l, std::chrono::seconds(10));
        } else {
            //Don't skip entierly, but throttle the loop to sane amount
            pluginLog(2, "Maintenance Thread - Skipping wait");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    pluginLog(2, "Maintenance Thread - Stopping maintenance thread, requested by user");
    pluginLog(2, "Maintenance Thread - Stopping inotify thread");
    for (auto &entry : this->watchDescriptors) {
        inotify_rm_watch(this->fd, entry.second);
    }
    close(this->fd);
    pluginLog(2, "Maintenance Thread - Inotify watch gracefully removed and inotify closed");
    pluginLog(2, "Maintenance Thread - Inotify thread stopped");
    this->mInotifyThread = NULL;
}

void LibraryController::inotifyThread() {
    int length, i = 0;
    char buffer[BUF_LEN];
    const struct inotify_event *event;
    this->watchDescriptors = std::map<std::filesystem::path, int>();
    this->watchDescriptorsPath = std::map<int, std::filesystem::path>();

    this->fd = inotify_init();
    if (fd < 0) {
        pluginLog(0, "Inotify Thread - Couldn't initialize inotify");
        return;
    } else {
        pluginLog(2, "Inotify Thread - Inotify initialized");
    }

    this->updateInotifyFolders();

    while (this->mMaintenanceThreadRun.load()) {
        length = read(this->fd, buffer, BUF_LEN);
        if (length < 0) {
            pluginLog(0, "Inotify Thread - Couldn't read from inotify");
            return;
        }
        
        i = 0;
        while (i<length) {
            event = (const struct inotify_event *) &buffer[i];
            if (event->len) {
                std::stringstream eventName;
                eventName << event->name;
                std::stringstream eventMask;
                eventMask << event->mask;
                std::filesystem::path path = this->watchDescriptorsPath[event->wd] / eventName.str();
                
                if (Filebrowser::VALID_EXTENSIONS.find(path.extension()) != Filebrowser::VALID_EXTENSIONS.end() || event->mask & IN_ISDIR) {
                    if (event->mask & IN_CREATE) {
                        if (event->mask & IN_ISDIR) {
                            pluginLog(2, "Inotify Thread - The directory " + (std::string)path + " was created.");
                            this->updateInotifyFolders();
                        } else {
                            pluginLog(2, "Inotify Thread - The file " + (std::string)path + " was created.");
                            //this->mImportQueue.push(path);
                            //this->mMaintenanceCtrl.notify_one();
                            //Use map, because it prevents duplicates
                            this->mRefreshQueueMutex.lock();
                            this->mRefreshQueue[path] = std::chrono::steady_clock::now();
                            this->mRefreshQueueMutex.unlock();
                            this->mMaintenanceCtrl.notify_one();
                        }
                    } else if (event->mask & IN_DELETE) {
                        if (event->mask & IN_ISDIR) {
                            pluginLog(2, "Inotify Thread - The directory " + (std::string)path + " was deleted.");
                            this->updateInotifyFolders();
                        } else {
                            pluginLog(2, "Inotify Thread - The file " + (std::string)path + " was deleted.");
                            this->mDeleteQueue.push(path);
                            this->mMaintenanceCtrl.notify_one();
                        }
                    } else if (event->mask & IN_CLOSE_WRITE) {
                        pluginLog(2, "Inotify Thread - The file " + (std::string)path + " was closed after being written.");
                        this->mImportQueue.push(path);
                        this->mMaintenanceCtrl.notify_one();
                    } else if (event->mask & IN_MODIFY) {
                        if (event->mask & IN_ISDIR) {
                            pluginLog(2, "Inotify Thread - The directory " + (std::string)path + " was modified.");
                            this->updateInotifyFolders();
                        } else {
                            //pluginLog(2, "Inotify Thread - The file " + (std::string)this->watchDescriptorsPath[event->wd] + "/" + eventName.str() + " was modified."); // This spams too much
                            this->mRefreshQueueMutex.lock();
                            this->mRefreshQueue[path] = std::chrono::steady_clock::now();
                            this->mRefreshQueueMutex.unlock();
                            this->mMaintenanceCtrl.notify_one();
                        }
                    } else if (event->mask & IN_MOVED_FROM) {
                        if (event->mask & IN_ISDIR) {
                            pluginLog(2, "Inotify Thread - The directory " + (std::string)path + " was moved from.");
                            this->updateInotifyFolders();
                        } else {
                            pluginLog(2, "Inotify Thread - The file " + (std::string)path + " was moved from.");
                            this->mDeleteQueue.push(path);
                            this->mMaintenanceCtrl.notify_one();
                        }
                    } else if (event->mask & IN_MOVED_TO) {
                        if (event->mask & IN_ISDIR) {
                            pluginLog(2, "Inotify Thread - The directory " + (std::string)path + " was moved to.");
                            this->updateInotifyFolders();
                        } else {
                            pluginLog(2, "Inotify Thread - The file " + (std::string)path + " was moved to.");
                            this->mImportQueue.push(path);
                            this->mMaintenanceCtrl.notify_one();
                        }
                    } else {
                        pluginLog(2, "Inotify Thread - Unknown event " + eventMask.str() + " for " + eventName.str());
                    }
                } else {
                    pluginLog(2, "Inotify Thread - Skipping " + path.string() + " because of invalid file extension");
                }
            }
            i += EVENT_SIZE + event->len;
        }
        //this->mMaintenanceCtrl.notify_one();
        //this->mImportPending = true;
    }
}

void LibraryController::updateInotifyFolders() {
    bool initial = false;
    if (this->watchDescriptors.size() == 0) {
        initial = true;
    }
    for (auto mlPath : this->mMediaLibrary->getSearchPaths()) {
        for (auto dirPath : Filebrowser::getDirectoryList(mlPath, false, false, true)) {
            if (this->watchDescriptors.find(dirPath) == this->watchDescriptors.end()) {
                if (!initial) pluginLog(2, "Inotify Thread - Adding watch for new directory.");
                int wd = inotify_add_watch(this->fd, dirPath.c_str(), IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
                if (wd < 0) {
                    pluginLog(0, "Inotify Thread - Couldn't add watch for " + dirPath.string());
                    continue;
                }
                this->watchDescriptors[dirPath] = wd;
                this->watchDescriptorsPath[wd] = dirPath;
            }
        }
    }
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
        row[mModelColumns.mColumnTooltip] = Utils::escapeTooltip(entry->Name);//+ " - " + entry->Artist;
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
        this->mMaintenanceCtrl.notify_one();
        this->mMaintenanceThread->join();
        this->mMaintenanceThread = NULL;
        this->mMaintenanceThreadRun = true;
    }
}

double LibraryController::getImportProgress() const {
    return this->mImportProgress;
}
bool LibraryController::getImportStatus() const {
    return this->mImportStatus;
}
double LibraryController::getMaintenanceProgress() const {
    return this->mMaintenanceProgress;
}
bool LibraryController::getMaintenanceStatus() const {
    return this->mMaintenanceStatus;
}