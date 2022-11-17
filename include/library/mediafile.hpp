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
    std::vector<std::string> Artists;
    std::string Album;
    std::string Genre;
    std::string Year;
    std::string Length;
    std::string FileFormat;
    time_t LastModified;
    CoverImage* Cover;
    std::hash<std::string> hash;

    std::string AlbumID;
    
    std::map<std::string, std::string> MetaData;

    template<class Archive>
    void serialize(Archive &a, const unsigned version){
        a & Path & Title & Artists & Album & Genre & Year & Length & FileFormat & LastModified & Cover & AlbumID & MetaData;
    }
};