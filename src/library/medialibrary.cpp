#include "medialibrary.hpp"

#include "string.h"
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include "covermediafile.hpp"
#include "coveralbum.hpp"
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

std::vector<Album> MediaLibrary::getAlbums() {
    std::vector<Album> albums = {};
    for (auto &album : this->mAlbumMap) {
        albums.push_back(*album.second);
    }
    return albums;
}

void MediaLibrary::addMediaFile(MediaFile* mediaFile) {
    if (this->mMediaFilesMap.find(mediaFile->Path) == this->mMediaFilesMap.end()) {
        this->mMediaFiles.push_back(mediaFile);
        this->mMediaFilesMap[mediaFile->Path] = mediaFile;
        this->addAlbum(mediaFile);
        mMediaFileCount++;
    }
}

void MediaLibrary::addMediaFile(std::filesystem::path path) {
    if (this->mMediaFilesMap.find(path) == this->mMediaFilesMap.end()) {
        MediaFile* mediaFile = new MediaFile(path);
        this->mMediaFiles.push_back(mediaFile);
        this->mMediaFilesMap[path] = mediaFile;
        this->addAlbum(mediaFile);
        mMediaFileCount++;
    }
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
        this->mAlbumMap[albumName] = album;

    
        Cache::Covers::CoverAlbum coverAlbum;
        album->CoverPixbuf = coverAlbum.getIcon(album, deadbeef->conf_get_int(ML_ICON_SIZE, 32));
        album->coverFound = coverAlbum.lastIconFound();

        mAlbumCount++;
    } else {
        this->mAlbumMap[albumName]->Length += mediaFile->Length;
        this->mAlbumMap[albumName]->MediaFiles.push_back(mediaFile);
    }
}

void MediaLibrary::loadCovers() {
    Cache::Covers::CoverMediaFile coverMediaFile;
    Cache::Covers::CoverAlbum coverAlbum;
    for (auto mediaFile : this->mMediaFiles) {
        mediaFile->CoverPixbuf = coverMediaFile.getIcon(mediaFile, deadbeef->conf_get_int(ML_ICON_SIZE, 32));
        mediaFile->coverFound = coverMediaFile.lastIconFound();

    }
    for (auto album : this->mAlbumMap) {
        album.second->CoverPixbuf = coverAlbum.getIcon(album.second, deadbeef->conf_get_int(ML_ICON_SIZE, 32));
        album.second->coverFound = coverAlbum.lastIconFound();
    }
}

std::string MediaLibrary::getStats() {
    std::string stats = "Albums: " + std::to_string(mAlbumCount) + " MediaFiles: " + std::to_string(mMediaFileCount);
    return stats;
}



MediaLibrary::~MediaLibrary() {
}