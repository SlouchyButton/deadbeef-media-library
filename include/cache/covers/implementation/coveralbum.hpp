#pragma once

#include <gtkmm.h>
#include <filesystem>

#include "cache.hpp"
#include "album.hpp"

class Cache::Covers::CoverAlbum {
public:
    CoverAlbum();
    ~CoverAlbum();

    Glib::RefPtr<Gdk::Pixbuf> getIcon(Album* album, uint size);

    bool lastIconFound();
private:
    bool status;
};