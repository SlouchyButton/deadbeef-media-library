#include "treefilebrowser.hpp"

#include <boost/algorithm/string.hpp>

#include "filebrowser.hpp"
#include "utils.hpp"
#include "plugin.hpp"
#include "addressbox.hpp"
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



TreeFilebrowser::~TreeFilebrowser() {
}