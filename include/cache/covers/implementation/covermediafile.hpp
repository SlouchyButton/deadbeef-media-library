#pragma once

#include <gtkmm.h>
#include <filesystem>

#include "cache.hpp"
#include "mediafile.hpp"

class Cache::Covers::CoverMediaFile {
public:
    CoverMediaFile();
    ~CoverMediaFile();

    Glib::RefPtr<Gdk::Pixbuf> getIcon(MediaFile* mediaFile, uint size);

    bool lastIconFound();
private:
    bool status;
};