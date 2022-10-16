#include "addressbox.hpp"

#include "filesystem"

#include "plugin.hpp"
#include "settings.hpp"
#include "utils.hpp"
#include "settings/window.hpp"

Addressbox::Addressbox() :
mRefreshButton(),
mSettingsButton() {
    this->mRefreshButton.signal_clicked().connect(sigc::mem_fun(*this, &Addressbox::on_go_button_click));
    this->mRefreshButton.set_margin_top(1);
    this->mRefreshButton.set_margin_bottom(1);
    this->mRefreshButton.set_margin_left(1);
    this->mRefreshButton.set_margin_right(1);
    this->mRefreshButton.set_image(*Gtk::manage(new Gtk::Image(Utils::getIconByName("view-refresh", 16))));
    
    this->mSettingsButton.signal_clicked().connect(sigc::mem_fun(*this, &Addressbox::on_settings_button_click));
    this->mSettingsButton.set_margin_top(1);
    this->mSettingsButton.set_margin_bottom(1);
    this->mSettingsButton.set_margin_left(1);
    this->mSettingsButton.set_margin_right(1);
    this->mSettingsButton.set_image(*Gtk::manage(new Gtk::Image(Utils::getIconByName("configure", 16))));

    this->pack_start(this->mSearchbar, true, true);
    this->pack_start(this->mRefreshButton, false, true);
    this->pack_start(this->mSettingsButton, false, true);
    this->pack_start(this->mProgressLabel, true, true);
}

void Addressbox::initialize(MediaLibrary* mediaLibrary, LibraryController* libraryController, SettingsWindow* settingsWindow, FilebrowserFilter* filter) {
    this->mMediaLibrary = mediaLibrary;
    this->mLibraryController = libraryController;
    this->mSettingsWindow = settingsWindow;
    this->mFilebrowserFilter = filter;

    this->mSearchbar.setTreeModelFilter(this->mFilebrowserFilter);
}

void Addressbox::updateProgress(bool status, float progress, std::string stats) {
    if (status == false) {
        if (this->mStatus == true) {
            this->mRefreshButton.set_image(*Gtk::manage(new Gtk::Image(Utils::getIconByName("view-refresh", 16))));
            this->mStatus = false;
        }
    } else {
        if (this->mStatus == false) {
            this->mRefreshButton.set_image(*Gtk::manage(new Gtk::Image(Utils::getIconByName("process-stop", 16))));
            this->mStatus = true;
        }
    }
    this->mSearchbar.set_progress_fraction(progress);
    this->mProgressLabel.set_text(stats);
}

void Addressbox::on_go_button_click() {
    if (this->mStatus == true) {
        pluginLog(2, "Addressbox - Import in progress, stopping on user request");
        this->mLibraryController->stopImport();
        return;
    } else {
        pluginLog(2, "Addressbox - Starting import");
        this->mLibraryController->startImport();
    }
}

void Addressbox::on_settings_button_click() {
    this->mSettingsWindow->show();
}

Addressbox::~Addressbox() {
}