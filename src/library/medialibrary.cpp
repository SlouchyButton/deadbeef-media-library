#include "medialibrary.hpp"

#include "string.h"
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include "plugin.hpp"
#include "settings.hpp"

MediaLibrary::MediaLibrary() {
    //Maybe do some initialization here, like loading from saved file
}

std::vector<MediaFile*> MediaLibrary::getMediaFiles() {
    return this->mMediaFiles;
}

std::vector<MediaFile*> MediaLibrary::getMediaFiles(std::string albumName){
    std::vector<MediaFile*> mediaFiles = {};
    for (auto &mediaFile : this->mMediaFiles) {
        if (mediaFile->Album == albumName) {
            mediaFiles.push_back(mediaFile);
        }
    }
    return mediaFiles;
}

std::vector<Album*> MediaLibrary::getAlbums() {
    std::vector<Album*> albums = {};
    for (auto &album : this->mAlbumMap) {
        albums.push_back(album.second);
    }
    return albums;
}

void MediaLibrary::addMediaFile(MediaFile* mediaFile) {
    if (this->mMediaFilesMap.find(mediaFile->Path) == this->mMediaFilesMap.end()) {
        this->mMediaFiles.push_back(mediaFile);
        this->mMediaFilesMap[mediaFile->Path] = mediaFile;
        this->addAlbum(mediaFile);
        this->mMediaFileCount++;
    }
}

void MediaLibrary::addMediaFile(std::filesystem::path path) {
    if (this->mMediaFilesMap.find(path) == this->mMediaFilesMap.end() && path.has_extension()) {
        MediaFile* mediaFile = new MediaFile(path);
        this->mMediaFiles.push_back(mediaFile);
        this->mMediaFilesMap[path] = mediaFile;
        this->addAlbum(mediaFile);
        this->mMediaFileCount++;
    }
}

void MediaLibrary::addSearchPath(std::filesystem::path path) {
    //Management not implemented yet, replace current search path
    /*if (std::find(this->mPaths.begin(), this->mPaths.end(), path) == this->mPaths.end()) {
        this->mPaths.push_back(path);
    }*/
    this->mPaths.clear();
    this->mPaths.push_back(path);
}

std::vector<std::filesystem::path> MediaLibrary::getSearchPaths() {
    std::vector<std::filesystem::path> paths;
    for (auto &path : this->mPaths) {
        paths.push_back(std::filesystem::path(path));
    }
    return paths;
}

void MediaLibrary::addAlbum(MediaFile* mediaFile) {
    std::string albumName = mediaFile->Album;
    if (this->mAlbumMap.find(albumName) == this->mAlbumMap.end()) {
        Album* album = new Album();
        album->Name = albumName;
        album->Artist = mediaFile->Artist;
        album->Genre = mediaFile->Genre;
        album->Year = mediaFile->Year;
        album->Length = mediaFile->Length;
        album->MediaFiles.push_back(mediaFile);

        album->Cover = new CoverImage(mediaFile->Cover->data, deadbeef->conf_get_int(ML_ICON_SIZE, 32));

        this->mAlbumMap[albumName] = album;

        this->mAlbumCount++;
    } else {
        this->mAlbumMap[albumName]->Length += mediaFile->Length;
        this->mAlbumMap[albumName]->MediaFiles.push_back(mediaFile);

        if (this->mAlbumMap[albumName]->Artist != mediaFile->Artist) {
            this->mAlbumMap[albumName]->Artist = "VA";
        }
    }
}

void MediaLibrary::loadCovers() {
    int size = deadbeef->conf_get_int(ML_ICON_SIZE, 128);
    for (auto mediaFile : this->mMediaFiles) {
        mediaFile->Cover->regeneratePixbuf(size);

    }
    for (auto album : this->mAlbumMap) {
        album.second->Cover->regeneratePixbuf(size);
    }
}

std::string MediaLibrary::getStats() {
    std::string stats = "Albums: " + std::to_string(this->mAlbumCount) + " MediaFiles: " + std::to_string(this->mMediaFileCount);
    return stats;
}



MediaLibrary::~MediaLibrary() {
}