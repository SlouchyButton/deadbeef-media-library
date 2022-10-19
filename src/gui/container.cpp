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
    this->mListStore = Gtk::ListStore::create(mModelColumns);
    this->mFilebrowserFilter = FilebrowserFilter::create(this->mListStore);
    this->mSettingsWindow = new SettingsWindow(&this->mLibraryController);

    this->mLibraryController.initialize(this->mListStore);
    this->mLibraryController.addCallback(std::bind(&Container::notify, this));

    this->mTreePopup.initialize(&this->mIconView, this->mFilebrowserFilter, &this->mLibraryController, &this->mAddressbox);
    this->mAddressbox.initialize(this->mLibraryController.mMediaLibrary, &this->mLibraryController, this->mSettingsWindow, this->mFilebrowserFilter.get());
    this->mIconView.initialize();
    //this->mTreeFilebrowser->initialize(&this->mIconView, &this->mAddressbox, &this->mMediaLibrary);

    //this->mTreeFilebrowser->setIconSize(deadbeef->conf_get_int(ML_ICON_SIZE, 128));

    this->mScrolledWindow.set_policy(Gtk::PolicyType::POLICY_NEVER, Gtk::PolicyType::POLICY_AUTOMATIC);
    this->mScrolledWindow.add(mIconView);


    this->pack_start(mAddressbox, false, true);
    this->pack_start(mScrolledWindow, true, true);
    
    Filebrowser::VALID_EXTENSIONS = Utils::createValidExtensions();

    this->initialize();
}

void Container::initialize() {
    // Create autofilter
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
        if (!doneNotify) {
            pluginLog(2, "Container - Workers done, setting model");
            this->mIconView.unset_model();
            this->mLibraryController.refreshModel();
            this->mIconView.set_model(this->mFilebrowserFilter);
            if (!this->mTreePopup.getCurrentPath().empty()) {
                this->mIconView.select_path(this->mTreePopup.getCurrentPath());
                this->mIconView.scroll_to_path(this->mTreePopup.getCurrentPath(), true, 0.5, 0.5);
            }
            this->mSettingsWindow->updatePaths(this->mLibraryController.mMediaLibrary->getSearchPaths());
            doneNotify = true;
        }
    } else {
        doneNotify = false;
    }
    this->mAddressbox.updateProgress(isImporting, this->mLibraryController.getImportProgress(), this->mLibraryController.mMediaLibrary->getStats());
    this->mSettingsWindow->updateProgress(isImporting, this->mLibraryController.getImportProgress(), this->mLibraryController.mMediaLibrary->getStats());
}

Container::~Container() {
}