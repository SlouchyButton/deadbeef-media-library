#include "treefilebrowser.hpp"

#include <boost/algorithm/string.hpp>

#include "filebrowser.hpp"
#include "utils.hpp"
#include "plugin.hpp"
#include "addressbox.hpp"
#include "coveralbum.hpp"

#include <fstream>

//include for object save and archive
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>


TreeFilebrowser::TreeFilebrowser() {
    this->set_column_types(mModelColumns);
    //this->treeStore = Gtk::TreeStore::create(mModelColumns);
    //mTreeView->set_model(this->treeStore);
}

Glib::RefPtr<TreeFilebrowser> TreeFilebrowser::create() {
    return Glib::RefPtr<TreeFilebrowser>(new TreeFilebrowser());
}

void TreeFilebrowser::setNeedleState(bool newState) {
    this->mIsNeedleSet = newState;
}

void TreeFilebrowser::initialize(Gtk::IconView *treeview, Addressbox *addressbox, MediaLibrary *medialibrary) {
    this->mIconView = treeview;
    this->mAddressbox = addressbox;
    this->mMediaLibrary = medialibrary;
}

void TreeFilebrowser::setIconSize(uint newIconSize) {
    this->mIconSize = newIconSize;
}

void TreeFilebrowser::setTreeRoot(std::filesystem::path newDirectory) {
    if (refreshLock) {
        return;
    }
    this->mTreeDirectory = newDirectory;
    this->refreshTree();
}

void TreeFilebrowser::refreshTree() {
    if (refreshLock) {
        return;
    }
    refreshLock = true;

    mRefreshThread = new std::thread(&TreeFilebrowser::refreshThread, this);
}

void TreeFilebrowser::refreshThread() {
    status = true;
    mIconView->unset_model();
    clear();

    //load mMediaLibrary from file
    std::cout << "Checking whether library is saved" << std::endl;
    std::string dbLibraryPath = deadbeef->get_system_dir(DDB_SYS_DIR_CACHE);
    dbLibraryPath += "/media-library/library.bin";
    std::filesystem::path libraryPath = dbLibraryPath;
    if (std::filesystem::exists(libraryPath) && !forceRefresh) {
        pluginLog(DDB_LOG_LAYER_INFO, "Library is saved, loading");
        std::ifstream ifs(libraryPath, std::ios::binary);
        boost::archive::binary_iarchive ia(ifs);
        ia >> mMediaLibrary;
        pluginLog(DDB_LOG_LAYER_INFO, "Library loaded, loading covers");
        mMediaLibrary->loadCovers();
    } else {
        pluginLog(DDB_LOG_LAYER_INFO, "Library is not saved, creating new");
        pluginLog(DDB_LOG_LAYER_INFO, "Loading tree structure");
        auto filelist = Filebrowser::getFileList(mTreeDirectory, true, false);
        std::size_t count = filelist.size();
        progressCount = count;
        threadProgress = 0;
        progressIteration = 0;
        if (count > 0) {
            for (auto &entry : filelist) {
                if (!mRefreshThreadRun.load()) {
                    pluginLog(DDB_LOG_LAYER_INFO, "Load canceled by user");
                    threadProgress = 1;
                    mAddressbox->notify();
                    refreshLock = false;
                    return;
                }
                threadProgress = progressIteration / progressCount;
                mAddressbox->notify();
                
                if (entry.is_directory() && !std::filesystem::is_empty(entry)) {
                    this->findChildren(entry.path());
                } else {
                    this->mMediaLibrary->addMediaFile(entry.path());
                    this->libraryStats = this->mMediaLibrary->getStats();
                }

                progressIteration++;
            }
        } else {
            TreeFilebrowser::fillEmptyRow(this);
        }
        pluginLog(DDB_LOG_LAYER_INFO, "Library created");
        //save mMediaLibrary to binary file
        pluginLog(DDB_LOG_LAYER_INFO, "Saving library to file");
        if (!std::filesystem::exists(libraryPath.parent_path())) {
            std::filesystem::create_directories(libraryPath.parent_path());
        }
        try {
            std::ofstream ofs(libraryPath, std::ios::binary);
            boost::archive::binary_oarchive oa(ofs);
            oa << mMediaLibrary;
            pluginLog(DDB_LOG_LAYER_INFO, "Library saved");
        } catch (const boost::archive::archive_exception &e) {
            std::string err = e.what();
            pluginLog(DDB_LOG_LAYER_INFO, "Error saving library to file " + libraryPath.string() + " with error: " + err);
        }
    }

    pluginLog(DDB_LOG_LAYER_INFO, "Library loaded");
    pluginLog(DDB_LOG_LAYER_INFO, "Filling model");
    for (auto &entry : this->mMediaLibrary->getAlbums()) {
        Gtk::TreeModel::iterator iter = append();
        Gtk::TreeRow row = *iter;
        row[mModelColumns.mColumnIcon] = entry->Cover->CoverPixbuf;
        row[mModelColumns.mColumnName] = entry->Name;
        row[mModelColumns.mColumnAlbumPointer] = entry;
        row[mModelColumns.mColumnTooltip] = Utils::escapeTooltip(entry->Artist);
        row[mModelColumns.mColumnVisibility] = true;
    }

    //mTreeView->set_model(this);
    refreshLock = false;
    forceRefresh = true;
    status = false;
    libraryStats = this->mMediaLibrary->getStats();
    pluginLog(DDB_LOG_LAYER_INFO, "Notifying dispatcher - task done");
    threadProgress = 1;
    mAddressbox->notify();
}

void TreeFilebrowser::findChildren(std::filesystem::path path) {
    auto filelist = Filebrowser::getFileList(path, false, false);
    std::size_t count = filelist.size();
    //progressCount += count;
    if (count > 0) {
        for (auto &entry : filelist) {
            if (!mRefreshThreadRun.load()) {
                pluginLog(DDB_LOG_LAYER_INFO, "Child load canceled by user");
                return;
            }

            if (entry.is_directory() && !std::filesystem::is_empty(entry)) {
                this->findChildren(entry.path());
            } else {
                this->mMediaLibrary->addMediaFile(entry.path());
                this->libraryStats = this->mMediaLibrary->getStats();
                mAddressbox->notify();
            }
            //progressIteration++;
        }
    }
}

void TreeFilebrowser::stopThread() {
    if(mRefreshThread != NULL) {
        mRefreshThreadRun = false;
        mRefreshThread->join();
        mRefreshThread = NULL;
        mRefreshThreadRun = true;
    }
}

void TreeFilebrowser::getProgress(float* progress) const {
    if (refreshLock) {
        *progress = threadProgress;
    } else {
        *progress = 1;
    }
}

void TreeFilebrowser::checkEmptyRoot() {
    if (this->children().empty()) {
        g_idle_add((GSourceFunc)&TreeFilebrowser::fillEmptyRow, this);
    }
}

int TreeFilebrowser::fillEmptyRow(void* data) {
    auto self = static_cast<TreeFilebrowser*>(data);
    auto row = *(self->append());
    Glib::RefPtr<Gdk::Pixbuf> emptyIcon;
    row[self->mModelColumns.mColumnIcon] = emptyIcon;
    row[self->mModelColumns.mColumnAlbumPointer] = NULL;
    row[self->mModelColumns.mColumnVisibility] = true;
    if (self->mIsNeedleSet) {
        row[self->mModelColumns.mColumnName] = "(EMPTY)";
        row[self->mModelColumns.mColumnTooltip] = "This directory is empty.";
    } else {
        row[self->mModelColumns.mColumnName] = "(NO RESULTS)";
        row[self->mModelColumns.mColumnTooltip] = "Try searching for something else.";
    }
    return 0;
}

TreeFilebrowser::~TreeFilebrowser() {
}