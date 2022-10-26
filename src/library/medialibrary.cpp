#include "medialibrary.hpp"

#include "string.h"
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>
#include "utils.hpp"

#include <iostream>
#include "plugin.hpp"
#include "settings.hpp"

MediaLibrary::MediaLibrary() {
    //Maybe do some initialization here, like loading from saved file
}

std::vector<MediaFile*> MediaLibrary::getMediaFiles() {
    return this->mMediaFiles;
}

std::map<std::string, MediaFile*> MediaLibrary::getMediaFilesMap() {
    return this->mMediaFilesMap;
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
        this->libraryDirty = true;
    }
}

void MediaLibrary::addMediaFile(std::filesystem::path path) {
    if (this->mMediaFilesMap.find(path) == this->mMediaFilesMap.end() && path.has_extension()) {
        MediaFile* mediaFile = new MediaFile(path);
        this->mMediaFiles.push_back(mediaFile);
        this->mMediaFilesMap[path] = mediaFile;
        this->addAlbum(mediaFile);
        this->libraryDirty = true;
    }
}

void MediaLibrary::removeMediaFile(MediaFile* mediaFile) {
    if (this->mMediaFilesMap.find(mediaFile->Path) != this->mMediaFilesMap.end()) {
        this->removeFromAlbum(mediaFile);
        this->mMediaFiles.erase(std::remove(this->mMediaFiles.begin(), this->mMediaFiles.end(), mediaFile), this->mMediaFiles.end());
        this->mMediaFilesMap.erase(mediaFile->Path);
        this->libraryDirty = true;
    }
}

void MediaLibrary::removeMediaFile(std::filesystem::path path) {
    if (this->mMediaFilesMap.find(path) != this->mMediaFilesMap.end()) {
        this->removeFromAlbum(this->mMediaFilesMap[path]);
        this->mMediaFiles.erase(std::remove(this->mMediaFiles.begin(), this->mMediaFiles.end(), this->mMediaFilesMap[path]), this->mMediaFiles.end());
        this->mMediaFilesMap.erase(path);
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

void MediaLibrary::sortMediaFiles() {
    //sort media files in albums by track number and then by title
    for (auto &album : this->mAlbumMap) {
        std::sort(album.second->MediaFiles.begin(), album.second->MediaFiles.end(), [](MediaFile* a, MediaFile* b) {
            if (a->MetaData["TRACKNUMBER"] == b->MetaData["TRACKNUMBER"]) {
                return a->Title < b->Title;
            }

            int aTrackNumber = 0;
            int bTrackNumber = 0;

            try {
                aTrackNumber = std::stoi(a->MetaData["TRACKNUMBER"]);
            } catch (std::invalid_argument& e) {
                pluginLog(1, "Track " + a->Title + " has invalid track number '" + a->MetaData["TRACKNUMBER"] + "' (in album " + a->Album + ")");
            }
            try {
                bTrackNumber = std::stoi(b->MetaData["TRACKNUMBER"]);
            } catch (std::invalid_argument& e) {
                pluginLog(1, "Track " + b->Title + " has invalid track number '" + b->MetaData["TRACKNUMBER"] + "' (in album " + b->Album + ")");
            }
            return aTrackNumber < bTrackNumber;
        });
    }
}

void MediaLibrary::sortAlbums() {
    //Not implemented yet
}

void MediaLibrary::clearLibrary() {
    this->mMediaFiles.clear();
    this->mMediaFilesMap.clear();
    this->mAlbumMap.clear();
    this->libraryDirty = true;
}

void MediaLibrary::addAlbum(MediaFile* mediaFile) {
    std::string albumId = mediaFile->Album;
    std::string albumArtist = mediaFile->Artists[0];
    std::cout << "Artists for " << mediaFile->Title << ": " << mediaFile->Artists.size() << std::endl;
    for (auto &artist : mediaFile->Artists) {
        std::cout << artist << "|";
    }
    std::cout << std::endl;
    bool albumExists = false;
    if (mediaFile->MetaData.find("ALBUM ARTIST") != mediaFile->MetaData.end()) {
        albumArtist = mediaFile->MetaData["ALBUM ARTIST"];
        albumId = mediaFile->Album + " - " + albumArtist;
        albumExists = !(this->mAlbumMap.find(albumId) == this->mAlbumMap.end());
    } else {
        albumId = mediaFile->Album + " - " + albumArtist;

        if (this->mAlbumMap.find(albumId) != this->mAlbumMap.end()) {
            albumExists = true;
        }
    }

    //try to lookup album with same cover art and name
    if (!albumExists) {
        for (auto &album : this->mAlbumMap) {
            if (album.second->Cover->hash == mediaFile->Cover->hash && album.second->Name == mediaFile->Album) {
                albumExists = true;
                albumId = album.second->Id;
                break;
            }
        }
    }
    
    if (!albumExists) {
        Album* album = new Album();
        album->Id = albumId;
        album->Name = mediaFile->Album;
        album->Artist = albumArtist;
        album->Genre = mediaFile->Genre;
        album->Year = mediaFile->Year;
        album->Length = mediaFile->Length;
        album->MediaFiles.push_back(mediaFile);

        album->Cover = mediaFile->Cover;

        this->mAlbumMap[albumId] = album;

    } else {
        this->mAlbumMap[albumId]->Length = mediaFile->Length;
        this->mAlbumMap[albumId]->MediaFiles.push_back(mediaFile);

        if (this->mAlbumMap[albumId]->Artist != albumArtist) {
            this->mAlbumMap[albumId]->Artist = "--VA--";
        }
    }
    this->libraryDirty = true;
}

void MediaLibrary::removeFromAlbum(MediaFile* mediaFile) {
    std::string albumArtist = mediaFile->MetaData.find("ALBUM ARTIST") != mediaFile->MetaData.end()? mediaFile->MetaData["ALBUM ARTIST"] : mediaFile->Artist;
    std::string albumId = mediaFile->Album + " - " + albumArtist;
    if (this->mAlbumMap.find(albumId) != this->mAlbumMap.end()) {
        //this->mAlbumMap[albumName]->Length -= mediaFile->Length;
        this->mAlbumMap[albumId]->MediaFiles.erase(std::remove(this->mAlbumMap[albumId]->MediaFiles.begin(), this->mAlbumMap[albumId]->MediaFiles.end(), mediaFile), this->mAlbumMap[albumId]->MediaFiles.end());
        if (this->mAlbumMap[albumId]->MediaFiles.size() == 0) {
            this->mAlbumMap.erase(albumId);
        }
    }
    this->libraryDirty = true;
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
    std::string stats = "Albums: " + std::to_string(this->mAlbumMap.size()) + " MediaFiles: " + std::to_string(this->mMediaFiles.size());
    return stats;
}

MediaLibrary::~MediaLibrary() {
}