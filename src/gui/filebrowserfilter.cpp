#include "filebrowserfilter.hpp"
#include "medialibrary.hpp"

#include <boost/algorithm/string.hpp>

#include <iostream>

FilebrowserFilter::FilebrowserFilter(Glib::RefPtr<Gtk::TreeModel> model) :
Gtk::TreeModelFilter(model) {
    this->set_visible_func(mem_fun(*this, &FilebrowserFilter::filter_func));
}

Glib::RefPtr<FilebrowserFilter> FilebrowserFilter::create(Glib::RefPtr<Gtk::ListStore> treefb) {
    return Glib::RefPtr<FilebrowserFilter>(new FilebrowserFilter(treefb));
}

void FilebrowserFilter::setNeedle(std::string newNeedle) {
    std::cout << "Lowering needle: " << newNeedle << std::endl;
    this->mNeedle = boost::to_lower_copy(newNeedle);
    std::cout << "Needle: " << this->mNeedle << std::endl;
    this->refilter();
}

void FilebrowserFilter::setModel(TreeFilebrowser* treefb) {
    this->mTreeFilebrowser = treefb;
}

bool FilebrowserFilter::filter_func(Gtk::TreeIter iter) {
    std::cout << "Filtering" << std::endl;
    if (this->mNeedle.empty()) {
        return true;
    }
    std::string currentTitle = boost::to_lower_copy(iter->get_value(mModelColumns.mColumnTitle));
    std::string currentSubtitle = boost::to_lower_copy(iter->get_value(mModelColumns.mColumnSubtitle));
    Album* currentAlbum = iter->get_value(mModelColumns.mColumnAlbumPointer);

    if (currentTitle.find(this->mNeedle) != std::string::npos ||
        currentSubtitle.find(this->mNeedle) != std::string::npos) {
        return true;
    }

    std::string currentMediaFileTitle;
    for (auto mediaFile : currentAlbum->MediaFiles) {
        currentMediaFileTitle = boost::to_lower_copy(mediaFile->Title);
        if (currentMediaFileTitle.find(this->mNeedle) != std::string::npos) {
            return true;
        }
    }

    return false;
}

FilebrowserFilter::~FilebrowserFilter() {
}