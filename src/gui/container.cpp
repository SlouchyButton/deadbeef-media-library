#include "container.hpp"

#include "settings.hpp"
#include "plugin.hpp"
#include "filebrowser.hpp"
#include "addressbox.hpp"
#include "utils.hpp"

void on_prop_changed(const Glib::ustring& sender_name, const Glib::ustring& signal_name, const Glib::VariantContainerBase& parameters);

Container::Container() :
    mAddressbox(),
    mScrolledWindow(),
    mIconView()
{
    mDispatcher.connect(sigc::mem_fun(*this, &Container::onNotify));

    //this->mTreeFilebrowser = TreeFilebrowser::create();
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

    prox = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BusType::BUS_TYPE_SESSION,
                                "org.freedesktop.portal.Desktop",
                                "/org/freedesktop/portal/desktop",
                                "org.freedesktop.portal.Settings",
                                Glib::RefPtr<Gio::DBus::InterfaceInfo>(), Gio::DBus::PROXY_FLAGS_NONE);
    
    
    Glib::Variant<Glib::ustring> name = Glib::Variant<Glib::ustring>::create(Glib::ustring("org.freedesktop.appearance"));
    Glib::Variant<Glib::ustring> key = Glib::Variant<Glib::ustring>::create(Glib::ustring("color-scheme"));
    std::vector<Glib::VariantBase> paramsVector = {name, key};
    
    Glib::VariantContainerBase params = Glib::VariantContainerBase::create_tuple(paramsVector);
    Glib::VariantContainerBase response = prox->call_sync(Glib::ustring("Read"), params);
    Glib::VariantBase value;
    response.get_child(value, 0);
    auto set = Gtk::Settings::get_default();
    Glib::PropertyProxy<bool> dark = set->property_gtk_application_prefer_dark_theme();
    if (value.print() == "<<uint32 1>>") {
        dark = true;
    } else {
        dark = false;
    }

    prox->signal_signal().connect(sigc::ptr_fun(&on_prop_changed));

    this->initialize();
}

void on_prop_changed(const Glib::ustring& sender_name, const Glib::ustring& signal_name, const Glib::VariantContainerBase& parameters) {
    
    Glib::VariantBase name;
    Glib::VariantBase value;

    if (parameters.get_n_children() < 3) {
        return;
    }

    parameters.get_child(name, 1);
    parameters.get_child(value, 2);
    
    if (name.print() != "'color-scheme'") {
        return;
    }
    auto set = Gtk::Settings::get_default();
    Glib::PropertyProxy<bool> dark = set->property_gtk_application_prefer_dark_theme();
    if (value.print() == "<uint32 1>") {
        dark = true;
    } else {
        dark = false;
    }
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
            std::string currentSearch = this->mFilebrowserFilter->getNeedle();
            this->mFilebrowserFilter->setNeedle("");
            this->mIconView.unset_model();
            this->mLibraryController.refreshModel();
            this->mIconView.set_model(this->mFilebrowserFilter);
            this->mFilebrowserFilter->setNeedle(currentSearch);
            if (!this->mTreePopup.getCurrentPath().empty()) {
                this->mIconView.select_path(this->mTreePopup.getCurrentPath());
                this->mIconView.scroll_to_path(this->mTreePopup.getCurrentPath(), true, 0.5, 0.5);
            }
            this->mSettingsWindow->updatePaths();
            doneNotify = true;
        }
    } else {
        doneNotify = false;
    }
    this->mAddressbox.updateProgress(isImporting, this->mLibraryController.getImportProgress(), this->mLibraryController.mMediaLibrary->getStats());
    this->mSettingsWindow->updateProgress(isImporting, this->mLibraryController.getImportProgress(), this->mLibraryController.mMediaLibrary->getStats());
}

Container::~Container() {
    this->mLibraryController.stopImport();
    this->mLibraryController.stopMaintenanceThread();
}