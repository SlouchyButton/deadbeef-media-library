#pragma once

#include <gtkmm.h>
#include <filesystem>

#include <iostream>

#include "coverimage.hpp"

class MediaFile {
public:
    MediaFile(std::filesystem::path path);
    MediaFile();
    ~MediaFile();

    std::string Path;
    std::string Title;
    std::string Artist;
    std::string Album;
    std::string Genre;
    std::string Year;
    std::string Length;
    std::string FileFormat;
    CoverImage* Cover;
    std::hash<std::string> hash;
    
    std::map<std::string, std::string> MetaData;

    template<class Archive>
    void serialize(Archive &a, const unsigned version){
        a & Path & Title & Artist & Album & Genre & Year & Length & FileFormat & MetaData & Cover;
    }

private:
    void loadMetaData();
};