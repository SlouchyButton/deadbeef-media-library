#pragma once

#include <gtkmm.h>
#include <filesystem>

#include "mediafile.hpp"
#include "album.hpp"
#include "coverimage.hpp"

#include <fstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/base_object.hpp>



class MediaLibrary {
public:
    MediaLibrary();
    ~MediaLibrary();

    std::vector<MediaFile*> getMediaFiles();
    std::map<std::string, MediaFile*> getMediaFilesMap();
    std::vector<MediaFile*> getMediaFiles(std::string albumName);
    std::vector<Album*> getAlbums();

    std::string getStats();

    void addMediaFile(MediaFile* mediaFile);
    void addMediaFile(std::filesystem::path path);
    //File will be reimported (if it exists - removed and imported again)
    void refreshMediaFile(std::filesystem::path path);
    void removeMediaFile(MediaFile* mediaFile);
    void removeMediaFile(std::filesystem::path path);

    void addSearchPath(std::filesystem::path path);
    void removeSearchPath(std::filesystem::path path);
    std::list<std::filesystem::path> getSearchPaths();

    void sortMediaFiles();
    void sortAlbums();

    void clearLibrary();

    void loadCovers();
    bool libraryDirty = false;
private:
    friend class boost::serialization::access;
    std::vector<MediaFile*> mMediaFiles;
    std::map<std::string, Album*> mAlbumMap;
    std::map<std::string, MediaFile*> mMediaFilesMap;
    std::list<std::string> mPaths;
    template<class Archive>
    void serialize(Archive &a, const unsigned version){
        a & mMediaFiles & mAlbumMap & mMediaFilesMap & mPaths;
    }
    
    void addAlbum(MediaFile* mediaFile);
    void removeFromAlbum(MediaFile* mediaFile);
};