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
    this->mNeedle = boost::to_lower_copy(newNeedle);
    this->refilter();
}

std::string FilebrowserFilter::getNeedle() {
    return this->mNeedle;
}

void FilebrowserFilter::setModel(TreeFilebrowser* treefb) {
    this->mTreeFilebrowser = treefb;
}

bool FilebrowserFilter::filter_func(Gtk::TreeIter iter) {
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

        std::string tagSearch = this->mNeedle;
        std::size_t tagSearchPos = tagSearch.find("#");
        if (tagSearchPos != std::string::npos) {
            bool foundAllTags = true;
            while (tagSearchPos != std::string::npos) {
                tagSearch = tagSearch.substr(tagSearchPos);
                
                std::size_t endCharPos = tagSearch.find(";");
                std::size_t valCharPos = tagSearch.find("=");
                if (endCharPos == std::string::npos || valCharPos == std::string::npos) {
                    break;
                }
                std::string searchedTag = tagSearch.substr(1, valCharPos-1);
                std::string searchedValue = tagSearch.substr(valCharPos+1, endCharPos-valCharPos-1);
                bool hasTag = false;
                for (auto tag : mediaFile->MetaData) {
                    std::string currentTag = boost::to_lower_copy(tag.first);
                    std::string currentValue = boost::to_lower_copy(tag.second);
                    if (currentTag == searchedTag) {
                        hasTag = true;
                    }
                    if (currentTag == searchedTag && currentValue.find(searchedValue) == std::string::npos) {
                        foundAllTags = false;
                    }
                }
                if (!hasTag) {
                    foundAllTags = false;
                }
                tagSearch = tagSearch.substr(endCharPos+1);
                tagSearchPos = tagSearch.find("#");
            }
            if (foundAllTags) {
                return true;
            }
        }
    }

    return false;
}

FilebrowserFilter::~FilebrowserFilter() {
}