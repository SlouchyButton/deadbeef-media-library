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

//0-Album name
//1-Artist name
//2-Album year
//3-Album genre
//Album rating
//Album play count
//Album last played
//4-Album date modified
std::vector<Album*> MediaLibrary::getAlbums() {
    std::vector<Album*> albums = {};
    for (auto &album : this->mAlbumMap) {
        albums.push_back(album.second);
    }
    int sortType = this->mSortType;
    std::sort(albums.begin(), albums.end(), [&sortType](Album* a, Album* b) {
        switch (sortType) {
            case 0:
                return a->Name < b->Name;
            case 1:
                return a->Artist < b->Artist;
            case 2:
                return a->Year > b->Year;
            case 3:
                return a->Genre < b->Genre;
            case 4:
                return a->MediaFiles[0]->LastModified > b->MediaFiles[0]->LastModified;
            default:
                return a->Name < b->Name;
        }
    });
    
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

void MediaLibrary::refreshMediaFile(std::filesystem::path path) {
    if (this->mMediaFilesMap.find(path) != this->mMediaFilesMap.end()) {
        MediaFile* mediaFile = this->mMediaFilesMap[path];
        int64_t newModifiedTime = std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(path).time_since_epoch()).count();
        if (newModifiedTime == mediaFile->LastModified) {
            return;
        }
        this->removeMediaFile(mediaFile);
    }
    this->addMediaFile(path);
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

            if (a->MetaData["TRACKNUMBER"] != "") {
                try {
                    aTrackNumber = std::stoi(a->MetaData["TRACKNUMBER"]);
                } catch (std::invalid_argument& e) {
                    pluginLog(1, "Track " + a->Title + " has invalid track number '" + a->MetaData["TRACKNUMBER"] + "' (in album " + a->Album + ")");
                }
            }
            
            if (b->MetaData["TRACKNUMBER"] != "") {
                try {
                    bTrackNumber = std::stoi(b->MetaData["TRACKNUMBER"]);
                } catch (std::invalid_argument& e) {
                    pluginLog(1, "Track " + b->Title + " has invalid track number '" + b->MetaData["TRACKNUMBER"] + "' (in album " + b->Album + ")");
                }
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
            if (album.second->Cover->data == mediaFile->Cover->data && album.second->Name == mediaFile->Album) {
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

        album->DateImported = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        this->mAlbumMap[albumId] = album;

    } else {
        this->mAlbumMap[albumId]->Length = mediaFile->Length;
        this->mAlbumMap[albumId]->MediaFiles.push_back(mediaFile);

        if (this->mAlbumMap[albumId]->Artist != albumArtist) {
            this->mAlbumMap[albumId]->Artist = "--VA--";
        }
    }

    mediaFile->AlbumID = albumId;

    this->libraryDirty = true;
}

void MediaLibrary::removeFromAlbum(MediaFile* mediaFile) {
    if (this->mAlbumMap.find(mediaFile->AlbumID) != this->mAlbumMap.end()) {
        //this->mAlbumMap[albumName]->Length -= mediaFile->Length;
        this->mAlbumMap[mediaFile->AlbumID]->MediaFiles.erase(std::remove(this->mAlbumMap[mediaFile->AlbumID]->MediaFiles.begin(), this->mAlbumMap[mediaFile->AlbumID]->MediaFiles.end(), mediaFile), this->mAlbumMap[mediaFile->AlbumID]->MediaFiles.end());
        if (this->mAlbumMap[mediaFile->AlbumID]->MediaFiles.size() == 0) {
            this->mAlbumMap.erase(mediaFile->AlbumID);
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