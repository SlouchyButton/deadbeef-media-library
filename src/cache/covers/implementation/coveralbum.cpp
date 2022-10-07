#include "coveralbum.hpp"

#include "readerfactory.hpp"
#include "utils.hpp"
#include "filebrowser.hpp"

#include "iostream"

using namespace Cache::Covers;

CoverAlbum::CoverAlbum() {
}

Glib::RefPtr<Gdk::Pixbuf> CoverAlbum::getIcon(Album* album, uint size) {
    /*Glib::RefPtr<Gdk::Pixbuf> icon;
    static Glib::RefPtr<Gdk::Pixbuf> lastIcon;
    static std::string lastDirectory;
    
    if (album->MediaFiles.front()->coverFound == true) {
        status = true;
        return album->MediaFiles.front()->CoverPixbuf;
    }
    
    status = false;
    return Utils::getIconByName("media-default-album", size);*/
}

bool CoverAlbum::lastIconFound() {
    return status;
}

CoverAlbum::~CoverAlbum() {
}
