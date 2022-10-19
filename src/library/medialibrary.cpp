#include "medialibrary.hpp"

#include "string.h"
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include "plugin.hpp"
#include "settings.hpp"

MediaLibrary::MediaLibrary() {
    this->mAlbumCount = 0;
    this->mMediaFileCount = 0;
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
        this->libraryDirty = true;
    }
}

void MediaLibrary::addMediaFile(std::filesystem::path path) {
    if (this->mMediaFilesMap.find(path) == this->mMediaFilesMap.end() && path.has_extension()) {
        MediaFile* mediaFile = new MediaFile(path);
        this->mMediaFiles.push_back(mediaFile);
        this->mMediaFilesMap[path] = mediaFile;
        this->addAlbum(mediaFile);
        this->mMediaFileCount++;
        this->libraryDirty = true;
    }
}

void MediaLibrary::removeMediaFile(MediaFile* mediaFile) {
    if (this->mMediaFilesMap.find(mediaFile->Path) != this->mMediaFilesMap.end()) {
        this->mMediaFiles.erase(std::remove(this->mMediaFiles.begin(), this->mMediaFiles.end(), mediaFile), this->mMediaFiles.end());
        this->mMediaFilesMap.erase(mediaFile->Path);
        this->removeFromAlbum(mediaFile);
        this->mMediaFileCount--;
        this->libraryDirty = true;
    }
}

void MediaLibrary::removeMediaFile(std::filesystem::path path) {
    if (this->mMediaFilesMap.find(path) != this->mMediaFilesMap.end()) {
        this->mMediaFiles.erase(std::remove(this->mMediaFiles.begin(), this->mMediaFiles.end(), this->mMediaFilesMap[path]), this->mMediaFiles.end());
        this->mMediaFilesMap.erase(path);
        this->removeFromAlbum(this->mMediaFilesMap[path]);
        this->mMediaFileCount--;
        this->libraryDirty = true;
    }
}

void MediaLibrary::addSearchPath(std::filesystem::path path) {
    if (std::find(this->mPaths.begin(), this->mPaths.end(), path) == this->mPaths.end()) {
        this->mPaths.push_back(path);
    }
    this->libraryDirty = true;
}

void MediaLibrary::removeSearchPath(std::filesystem::path path) {
    this->mPaths.remove(path);
    this->libraryDirty = true;
}

std::list<std::filesystem::path> MediaLibrary::getSearchPaths() {
    std::list<std::filesystem::path> paths;
    for (auto &path : this->mPaths) {
        paths.push_back(std::filesystem::path(path));
    }
    return paths;
}

void MediaLibrary::clearLibrary() {
    this->mMediaFiles.clear();
    this->mMediaFilesMap.clear();
    this->mAlbumMap.clear();
    this->mAlbumCount = 0;
    this->mMediaFileCount = 0;
    this->libraryDirty = true;
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
    this->libraryDirty = true;
}

void MediaLibrary::removeFromAlbum(MediaFile* mediaFile) {
    std::string albumName = mediaFile->Album;
    if (this->mAlbumMap.find(albumName) != this->mAlbumMap.end()) {
        //this->mAlbumMap[albumName]->Length -= mediaFile->Length;
        this->mAlbumMap[albumName]->MediaFiles.erase(std::remove(this->mAlbumMap[albumName]->MediaFiles.begin(), this->mAlbumMap[albumName]->MediaFiles.end(), mediaFile), this->mAlbumMap[albumName]->MediaFiles.end());
        if (this->mAlbumMap[albumName]->MediaFiles.size() == 0) {
            this->mAlbumMap.erase(albumName);
            this->mAlbumCount--;
        }
    }
    this->libraryDirty = true;
}

void MediaLibrary::loadCovers() {
    int size = deadbeef->conf_get_int(ML_ICON_SIZE, 128);
    for (auto mediaFile : this->mMediaFiles) {
        mediaFile->Cover->regeneratePixbuf(size);
        this->mMediaFileCount++;

    }
    for (auto album : this->mAlbumMap) {
        album.second->Cover->regeneratePixbuf(size);
        this->mAlbumCount++;
    }
}

std::string MediaLibrary::getStats() {
    std::string stats = "Albums: " + std::to_string(this->mAlbumMap.size()) + " MediaFiles: " + std::to_string(this->mMediaFiles.size());
    return stats;
}

MediaLibrary::~MediaLibrary() {
}