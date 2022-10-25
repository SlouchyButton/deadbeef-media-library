#include "treepopup.hpp"

#include <filesystem>

#include "plugin.hpp"
#include "settings.hpp"

TreePopup::TreePopup() {
    this->mMenu = Gio::Menu::create();

    this->mMenu->append("_Add to current playlist", "popup.add");
    this->mMenu->append("_Replace current playlist", "popup.replace");
    this->mMenu->append("_Open containing folder", "popup.open");
    this->mMenu->append("_Refresh", "popup.refresh");
    this->mMenu->append("_Print metadata", "popup.metadata");

    this->mActionGroup = Gio::SimpleActionGroup::create();

    this->mActionGroup->add_action("add", sigc::mem_fun(*this, &TreePopup::popup_add));
    this->mActionGroup->add_action("replace", sigc::mem_fun(*this, &TreePopup::popup_replace));
    this->mActionGroup->add_action("open", sigc::mem_fun(*this, &TreePopup::popup_open_folder));
    this->mActionGroup->add_action("refresh", sigc::mem_fun(*this, &TreePopup::popup_refresh));
    this->mActionGroup->add_action("metadata", sigc::mem_fun(*this, &TreePopup::popup_metadata));

    this->insert_action_group("popup", this->mActionGroup);
    this->bind_model(mMenu, true);
}

void TreePopup::initialize(Gtk::IconView* iconView, Glib::RefPtr<FilebrowserFilter> model, LibraryController* libraryController, Addressbox* addressbox /*TreeFilebrowser* treefb, FilebrowserFilter* filter*/) {
    this->mIconView = iconView;
    //this->mTreeFilebrowser = treefb;
    //this->mFilter = filter;
    this->mModel = model;
    this->mAddressbox = addressbox;
    this->mLibraryController = libraryController;

    this->mIconView->signal_button_press_event().connect(sigc::mem_fun(*this, &TreePopup::on_click), false);

    this->show_all();
}

Gtk::TreeModel::Path TreePopup::getCurrentPath() const {
    return this->mPath;
}

bool TreePopup::on_click(GdkEventButton* event) {
    this->mPath = this->mIconView->get_path_at_pos(event->x, event->y);

    if (this->mPath.empty()) {
        return false;
    }

    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        this->mIconView->select_path(this->mPath);
        this->popup_at_pointer(reinterpret_cast<GdkEvent*>(event));
    } else if (event->type == GDK_2BUTTON_PRESS && event->button == 1) {
        Gtk::TreeModel::iterator iter = this->mModel->get_iter(this->mPath);
        const Gtk::TreeRow row = *iter;
        mDataHolder.albums = {row[mModelColumns.mColumnAlbumPointer]};
        mDataHolder.replace = deadbeef->conf_get_int(ML_DOUBLECLICK_REPLACE, 1);
        intptr_t tid = deadbeef->thread_start(TreePopup::threadedAddToPlaylist, static_cast<void*>(&mDataHolder));
        deadbeef->thread_detach(tid);
    } else {
        this->mIconView->select_path(this->mPath);
    }

    return true;
}

void TreePopup::popup_add() {
        Gtk::TreeModel::iterator iter = this->mModel->get_iter(this->mPath);
        const Gtk::TreeRow row = *iter;
        mDataHolder.albums = {row[mModelColumns.mColumnAlbumPointer]};
        mDataHolder.replace = false;
        intptr_t tid = deadbeef->thread_start(TreePopup::threadedAddToPlaylist, static_cast<void*>(&mDataHolder));
        deadbeef->thread_detach(tid);
}

void TreePopup::popup_replace() {
        Gtk::TreeModel::iterator iter = this->mModel->get_iter(this->mPath);
        const Gtk::TreeRow row = *iter;
        mDataHolder.albums = {row[mModelColumns.mColumnAlbumPointer]};
        mDataHolder.replace = true;
        intptr_t tid = deadbeef->thread_start(TreePopup::threadedAddToPlaylist, static_cast<void*>(&mDataHolder));
        deadbeef->thread_detach(tid);
}

void TreePopup::threadedAddToPlaylist(void* ctx) {
    auto holder = (TreePopup::structAddToPlaylist*)(ctx);
    TreePopup::addToPlaylist(holder->albums, holder->replace);
}

void TreePopup::addToPlaylist(std::vector<Album*> albums, bool replace) {
    ddb_playlist_t* plt = deadbeef->plt_get_curr();

    if (replace) {
        deadbeef->plt_select_all(plt);
        deadbeef->plt_delete_selected(plt);
    }

    for (auto &album : albums) {

        deadbeef->plt_add_files_begin(plt, 0);

        
        for (auto &file : album->MediaFiles) {
            deadbeef->plt_add_file2(0, plt, file->Path.c_str(), NULL, NULL);
        }

        deadbeef->plt_add_files_end(plt, 0);
        deadbeef->plt_modified(plt);
        deadbeef->plt_save_config(plt);
        deadbeef->conf_save();
    }

    if (replace) {
        deadbeef->sendmessage(DB_EV_PLAY_NUM, 0, 0, 0);
    } else {
        if (deadbeef->playback_get_pos() == 0.0) {
            deadbeef->sendmessage(DB_EV_NEXT, 0, 0, 0);
        }
    }
}

void TreePopup::popup_open_folder() {
    Gtk::TreeModel::iterator iter = this->mModel->get_iter(this->mPath);
    const Gtk::TreeRow row = *iter;
    Album* album = row[mModelColumns.mColumnAlbumPointer];
    std::filesystem::path path = album->MediaFiles[0]->Path;
    path = path.parent_path();
    std::string URI = "file:///" + path.string();

    GError *error = NULL;
    if (!g_app_info_launch_default_for_uri(URI.c_str(), NULL, &error)) {
        g_warning("Failed to oepn uri: %s", error->message);
    }
}

void TreePopup::popup_refresh() {
    this->mAddressbox->on_go_button_click();
}

void TreePopup::popup_metadata() {
    Gtk::TreeModel::iterator iter = this->mModel->get_iter(this->mPath);
    const Gtk::TreeRow row = *iter;
    Album* album = row[mModelColumns.mColumnAlbumPointer];
    for (auto &file : album->MediaFiles) {
        for (auto &tag : file->MetaData) {
            std::cout << tag.first << ": " << tag.second << std::endl;
        }
    }
}

TreePopup::~TreePopup() {
}