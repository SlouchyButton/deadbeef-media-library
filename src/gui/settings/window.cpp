#include "window.hpp"

#include <iostream>

SettingsWindow::SettingsWindow(LibraryController* controller) :
    mFileChooserButton("Choose a folder"),
    mAddPathButton("Add path"),
    mImportButton("Import"),
    mCleanImportButton("Clean import"),
    mRemovePathButton("Remove path"),
    mStatusbar(),
    mProgressBar()
{
    this->set_default_size(550, 300);
    this->set_title("Library Manager");

    this->mListStore = Gtk::ListStore::create(mModelColumns);

    this->mController = controller;

    Gtk::Box* mMainBox = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
    Gtk::Grid* mGrid = new Gtk::Grid();
    Gtk::Box* mButtonBox = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Box* mFileSelectBox = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);

    
    this->mPathsView = new Gtk::TreeView();
    this->mPathsView->append_column("Paths", mModelColumns.mColumnPath);
    mPathsView->set_model(mListStore);

    mFileSelectBox->set_halign(Gtk::ALIGN_START);
    mFileSelectBox->set_margin_top(5); mFileSelectBox->set_margin_bottom(5); mFileSelectBox->set_margin_left(5); mFileSelectBox->set_margin_right(5);
    mButtonBox->set_margin_top(5); mButtonBox->set_margin_bottom(5); mButtonBox->set_margin_left(5); mButtonBox->set_margin_right(5);

    this->mFileChooserButton.set_margin_right(5);

    this->mAddPathButton.set_margin_top(1); this->mAddPathButton.set_margin_bottom(1); this->mAddPathButton.set_margin_left(1); this->mAddPathButton.set_margin_right(1);
    this->mAddPathButton.signal_clicked().connect(sigc::mem_fun(*this, &SettingsWindow::on_addPath_button_click));

    this->mRemovePathButton.set_margin_top(1); this->mRemovePathButton.set_margin_bottom(1); this->mRemovePathButton.set_margin_left(1); this->mRemovePathButton.set_margin_right(1);
    this->mRemovePathButton.signal_clicked().connect(sigc::mem_fun(*this, &SettingsWindow::on_removePath_button_click));

    this->mImportButton.set_margin_top(1); this->mImportButton.set_margin_bottom(1); this->mImportButton.set_margin_left(1); this->mImportButton.set_margin_right(5);
    this->mImportButton.signal_clicked().connect(sigc::mem_fun(*this, &SettingsWindow::on_import_button_click));
    
    this->mCleanImportButton.set_margin_top(1); this->mCleanImportButton.set_margin_bottom(1); this->mCleanImportButton.set_margin_left(1); this->mCleanImportButton.set_margin_right(5);
    this->mCleanImportButton.signal_clicked().connect(sigc::mem_fun(*this, &SettingsWindow::on_clean_import_button_click));

    //set css for progress bar
    Glib::RefPtr<Gtk::CssProvider> cssProvider = Gtk::CssProvider::create();
    cssProvider->load_from_data("progress, trough { min-height: 10px; }");
    this->mProgressBar.get_style_context()->add_provider(cssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    this->mProgressBar.set_margin_right(5);
    this->mProgressBar.set_margin_bottom(9);

    this->mProgressLabel.set_margin_right(10);

    //expand file select box
    mFileChooserButton.set_hexpand(true);
    this->mPathsView->set_hexpand(true);
    this->mPathsView->set_vexpand(true);
    mButtonBox->set_hexpand(true);

    this->mStatusbar.set_margin_left(0);
    this->mStatusbar.set_margin_right(0);
    this->mStatusbar.set_margin_top(0);
    this->mStatusbar.set_margin_bottom(0);

    this->mFileChooserButton.set_action(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);

    //align widgets in statusbar to the left
    this->mStatusbar.set_halign(Gtk::ALIGN_START);
    mStatusbar.pack_start(mProgressBar, false, false, 0);
    mStatusbar.pack_start(mProgressLabel, false, false, 0);
    mStatusbar.pack_start(mStatusLabel, false, false, 0);

    mMainBox->pack_start(*mGrid, true, true);
    mMainBox->pack_start(mStatusbar, false, false);

    mButtonBox->pack_start(mImportButton, false, false);
    mButtonBox->pack_start(mCleanImportButton, false, false);
    mButtonBox->pack_start(mRemovePathButton, false, false);

    mFileSelectBox->pack_start(mFileChooserButton, false, false);
    mFileSelectBox->pack_start(mAddPathButton, false, false);

    mGrid->attach(*mFileSelectBox, 0, 0, 2);
    mGrid->attach(*mPathsView, 0, 1, 2);
    mGrid->attach(*mButtonBox, 0, 2, 2);
    
    this->add(*mMainBox);

    this->show_all_children();
}

void SettingsWindow::updatePaths(std::list<std::filesystem::path> paths) {
    this->mListStore->clear();
    Gtk::TreeModel::iterator iter;
    for (std::filesystem::path &path : paths) {
        iter = this->mListStore->append();
        Gtk::TreeModel::Row row = *iter;
        row[mModelColumns.mColumnPath] = path.string();
    }
}

void SettingsWindow::updateProgress(bool status, double progress, std::string stats) {
    if (status == false) {
        if (this->mStatus == true) {
            this->mImportButton.set_label("Import");
            this->mStatusLabel.set_text("");
            this->mStatus = false;
        }
    } else {
        if (this->mStatus == false) {
            this->mImportButton.set_label("Stop");
            this->mStatusLabel.set_text("Import in progress");
            this->mStatus = true;
        }
    }
    if (this->mProgressBar.get_fraction() != progress) {
        this->mProgressBar.set_fraction(progress);
    }
    this->mProgressLabel.set_text(stats);
}

void SettingsWindow::on_addPath_button_click() {
    std::filesystem::path path = this->mFileChooserButton.get_filename();
    this->mController->mMediaLibrary->addSearchPath(path);
    this->updatePaths(this->mController->mMediaLibrary->getSearchPaths());
}

void SettingsWindow::on_removePath_button_click() {
    Gtk::TreeModel::iterator iter = this->mPathsView->get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        std::string path = row[mModelColumns.mColumnPath];
        this->mController->mMediaLibrary->removeSearchPath(path);
        this->updatePaths(this->mController->mMediaLibrary->getSearchPaths());
    }
}

void SettingsWindow::on_import_button_click() {
    this->mController->startImport();
}

void SettingsWindow::on_clean_import_button_click() {
    this->mController->clearLibrary();
    this->mController->startImport();
}

SettingsWindow::~SettingsWindow() {

}