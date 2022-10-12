#include "container.hpp"

#include "settings.hpp"
#include "plugin.hpp"
#include "filebrowser.hpp"
#include "addressbox.hpp"
#include "utils.hpp"

Container::Container() :
    mAddressbox(),
    mScrolledWindow(),
    mIconView()
{
    mDispatcher.connect(sigc::mem_fun(*this, &Container::onNotify));

    this->mTreeFilebrowser = TreeFilebrowser::create();
    this->mMediaLibrary = MediaLibrary();
    this->mListStore = Gtk::ListStore::create(mModelColumns);

    this->mLibraryController.initialize(&this->mMediaLibrary, this->mListStore);
    this->mLibraryController.addCallback(std::bind(&Container::notify, this));

    this->mTreePopup.initialize(&this->mIconView, this->mListStore, &this->mAddressbox);
    this->mAddressbox.initialize(&this->mMediaLibrary, &this->mLibraryController);
    this->mIconView.initialize();
    //this->mTreeFilebrowser->initialize(&this->mIconView, &this->mAddressbox, &this->mMediaLibrary);

    //this->mTreeFilebrowser->setIconSize(deadbeef->conf_get_int(ML_ICON_SIZE, 128));
    this->mFilebrowserFilter = FilebrowserFilter::create(this->mListStore);
    this->mSearchbar.setTreeModelFilter(this->mFilebrowserFilter.get());

    this->mScrolledWindow.set_policy(Gtk::PolicyType::POLICY_NEVER, Gtk::PolicyType::POLICY_AUTOMATIC);
    this->mScrolledWindow.add(mIconView);


    this->pack_start(mSearchbar, false, true);
    this->pack_start(mAddressbox, false, true);
    this->pack_start(mScrolledWindow, true, true);

    this->initialize();
}

void Container::initialize() {
    // Create autofilter
    Filebrowser::VALID_EXTENSIONS = Utils::createValidExtensions();
}

void Container::notify() {
    //pluginLog(2, "Container - Dispatcher notified");
    mDispatcher.emit();
}

void Container::buildTreeview() {

}

void Container::onNotify() {
    //pluginLog(2, "Container - Update request received from dispatcher");
    bool isImporting = this->mLibraryController.getImportStatus();
    bool isMaintaining = this->mLibraryController.getMaintenanceStatus();
    bool workersDone = !isImporting && !isMaintaining;
    if (workersDone) {
        pluginLog(2, "Container - Workers done, setting model");
        /*for (auto row: this->mListStore->children()) {
            std::cout << row.get_value(mModelColumns.mColumnTitle) << std::endl;
        }*/
        this->mIconView.set_model(this->mFilebrowserFilter);
    } else {
        this->mIconView.unset_model();
    }
    this->mAddressbox.updateProgress(isImporting, this->mLibraryController.getImportProgress(), this->mMediaLibrary.getStats());
}

Container::~Container() {
}