#pragma once

#include <gtkmm.h>
#include <filesystem>

#include <iostream>

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
    Glib::RefPtr<Gdk::Pixbuf> CoverPixbuf;
    bool coverFound;
    std::hash<std::string> hash;
    
    std::map<std::string, std::string> MetaData;

    template<class Archive>
    void serialize(Archive &a, const unsigned version){
        a & Path & Title & Artist & Album & Genre & Year & Length & FileFormat & MetaData;
    }

private:
    void loadMetaData();
};