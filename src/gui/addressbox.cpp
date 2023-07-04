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
    this->mRefreshButton.set_image(*Gtk::manage(new Gtk::Image(Utils::getIconByName("view-refresh-symbolic", 16))));
    
    this->mSettingsButton.signal_clicked().connect(sigc::mem_fun(*this, &Addressbox::on_settings_button_click));
    this->mSettingsButton.set_margin_top(1);
    this->mSettingsButton.set_margin_bottom(1);
    this->mSettingsButton.set_margin_left(1);
    this->mSettingsButton.set_margin_right(1);
    this->mSettingsButton.set_image(*Gtk::manage(new Gtk::Image(Utils::getIconByName("open-menu-symbolic", 16))));

    this->mSortCombo.set_margin_top(1);
    this->mSortCombo.set_margin_bottom(1);
    this->mSortCombo.set_margin_left(1);
    this->mSortCombo.set_margin_right(1);
    this->mSortCombo.append("Album name");
    this->mSortCombo.append("Artist name");
    this->mSortCombo.append("Album year");
    this->mSortCombo.append("Album genre");
    //this->mSortCombo.append("Album rating");
    //this->mSortCombo.append("Album play count");
    //this->mSortCombo.append("Album last played");
    this->mSortCombo.append("Album date modified");
    this->mSortCombo.set_active(0);
    this->mSortCombo.signal_changed().connect(sigc::mem_fun(*this, &Addressbox::on_sort_combo_changed));

    this->pack_start(this->mSearchbar, true, true);
    this->pack_start(this->mSortCombo, false, false);
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

void Addressbox::updateProgress(bool status, double progress, std::string stats) {
    if (status == false) {
        if (this->mStatus == true) {
            this->mRefreshButton.set_image(*Gtk::manage(new Gtk::Image(Utils::getIconByName("view-refresh-symbolic", 16))));
            this->mStatus = false;
        }
    } else {
        if (this->mStatus == false) {
            this->mRefreshButton.set_image(*Gtk::manage(new Gtk::Image(Utils::getIconByName("process-stop-symbolic", 16))));
            this->mStatus = true;
        }
    }
    if (this->mSearchbar.get_progress_fraction() != progress) {
        this->mSearchbar.set_progress_fraction(progress);
    }
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

void Addressbox::on_sort_combo_changed() {
    int sortType = this->mSortCombo.get_active_row_number();
    this->mLibraryController->setSortType(sortType);
}

Addressbox::~Addressbox() {
}