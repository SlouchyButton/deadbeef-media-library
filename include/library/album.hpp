#pragma once

#include <gtkmm.h>
#include "mediafile.hpp"
#include "coverimage.hpp"

class Album {
public:
    Album();
    ~Album();

    std::string Id;
    std::string Name;
    std::string Artist;
    std::string Genre;
    std::string Year;
    std::string Length;
    CoverImage* Cover;
    std::vector<MediaFile*> MediaFiles;
    time_t DateImported;
    //int PlayCount;
    
    template<class Archive>
    void serialize(Archive &a, const unsigned version){
        a & Id & Name & Artist & Genre & Year & Length & MediaFiles & Cover & DateImported /* & PlayCount*/;
    }
private:
};