#pragma once

#include <gtkmm.h>
#include "mediafile.hpp"

//class MediaFile;

class Album {
public:
    Album();
    ~Album();

    std::string Name;
    std::string Artist;
    std::string Genre;
    std::string Year;
    std::string Length;
    Glib::RefPtr<Gdk::Pixbuf> CoverPixbuf;
    bool coverFound;
    std::vector<MediaFile*> MediaFiles;
    
    template<class Archive>
    void serialize(Archive &a, const unsigned version){
        a & Name & Artist & Genre & Year & Length & MediaFiles;
    }
private:
};