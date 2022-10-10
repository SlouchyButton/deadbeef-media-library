#include "addressbox.hpp"

#include "filesystem"

#include "plugin.hpp"
#include "settings.hpp"
#include "utils.hpp"

Addressbox::Addressbox() :
mGoButton() {
    this->mGoButton.signal_clicked().connect(sigc::mem_fun(*this, &Addressbox::on_go_button_click));
    this->mGoButton.set_margin_top(1);
    this->mGoButton.set_margin_bottom(1);
    this->mGoButton.set_margin_left(1);
    this->mGoButton.set_margin_right(1);
    this->mGoButton.set_image(*Gtk::manage(new Gtk::Image(Utils::getIconByName("go-next", 16))));

    this->pack_start(this->mAddressBar, true, true);
    this->pack_start(this->mGoButton, false, true);
    this->pack_start(this->mProgressLabel, true, true);
}

void Addressbox::initialize(MediaLibrary* mediaLibrary, LibraryController* libraryController) {
    this->mMediaLibrary = mediaLibrary;
    this->mLibraryController = libraryController;

    std::string defaultDir = Glib::get_user_special_dir(Glib::UserDirectory::USER_DIRECTORY_MUSIC);
    deadbeef->conf_lock();
    auto address = deadbeef->conf_get_str_fast(ML_DEFAULT_PATH, defaultDir.c_str());
    deadbeef->conf_unlock();
    this->setAddress(address);
}

void Addressbox::setAddress(std::string newAddress) {
    this->mAddressBar.set_text(newAddress);
    //this->on_go_button_click();
}

void Addressbox::updateProgress(bool status, float progress, std::string stats) {
    //pluginLog(2, "Addressbox - Updating progress" + std::to_string(progress) + " status: " + std::to_string(status) + " my status: " + std::to_string(this->mStatus) + " stats: " + stats);
    if (status == false) {
        if (this->mStatus == true) {
            this->mAddressBar.set_sensitive(true);
            this->mGoButton.set_image(*Gtk::manage(new Gtk::Image(Utils::getIconByName("go-next", 16))));
            this->mStatus = false;
        }
        this->mAddressBar.set_progress_fraction(0);
    } else {
        if (this->mStatus == false) {
            this->mAddressBar.set_sensitive(false);
            this->mGoButton.set_image(*Gtk::manage(new Gtk::Image(Utils::getIconByName("process-stop", 16))));
            this->mStatus = true;
        }
        this->mAddressBar.set_progress_fraction(progress);
    }
    this->mProgressLabel.set_text(stats);
}

std::string Addressbox::getAddress() {
    return this->mAddress;
}

void Addressbox::on_go_button_click() {
    if (this->mStatus == true) {
        pluginLog(2, "Addressbox - Import in progress, stopping on user request");
        this->mLibraryController->stopImport();
        return;
    } else {
        pluginLog(2, "Addressbox - Saving path");
        this->mAddress = this->mAddressBar.get_text();
        this->mAddress = this->makeValidPath(this->mAddress);
        deadbeef->conf_set_str(ML_DEFAULT_PATH, this->mAddress.c_str());
        this->mAddressBar.set_text(this->mAddress);
        if (std::filesystem::exists(this->mAddress) && std::filesystem::is_directory(this->mAddress)) {
            this->mMediaLibrary->addSearchPath(this->mAddress);
        }
        pluginLog(2, "Addressbox - Starting import of " + this->mAddress);
        this->mLibraryController->startImport();
    }
}

std::string Addressbox::makeValidPath(std::string path) {
    uint count = 0;
    for (uint i = path.size() - 1; i >= 0; i--) {
        if (path[i] == '/') {
            count++;
        } else {
            break;
        }
    }

    path.resize(path.size() - count);
    return path;
}

Addressbox::~Addressbox() {
}