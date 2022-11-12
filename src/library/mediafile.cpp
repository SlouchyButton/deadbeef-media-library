#include <fileref.h>
#include <tag.h>
#include <tpropertymap.h>

#include "mediafile.hpp"
#include "utils.hpp"
#include "plugin.hpp"
#include "settings.hpp"
#include "readerfactory.hpp"

MediaFile::MediaFile(std::filesystem::path path) {
    TagLib::FileRef f(path.c_str());

    std::string folderName = path.parent_path().filename().string();

    if(!f.isNull() && f.tag()) {
        TagLib::Tag *tag = f.tag();
        this->Title = tag->title().isEmpty()? path.filename().string() : tag->title().toCString(true);
        this->Artist = tag->artist().isEmpty()? "Unknown artist" : tag->artist().toCString(true);
        
        TagLib::PropertyMap properties = f.file()->properties();
        TagLib::StringList artists = properties["ARTIST"];
        if (artists.isEmpty()) {
            this->Artists.push_back("Unknown Artist");
        } else {
            for (auto &artist : artists) {
                this->Artists.push_back(artist.toCString(true));
            }
        }

        this->Album = tag->album().isEmpty()? folderName : tag->album().toCString(true);
        this->Genre = tag->genre().isEmpty()? "Unkown genre" : tag->genre().toCString(true);
        this->Year = tag->year() == 0? "Unkown year" : std::to_string(tag->year());
        this->FileFormat = path.extension().c_str();
        for (auto &property : tag->properties()) {
            this->MetaData[property.first.toCString(true)] = property.second.front().toCString(true);
        }
    } else {
        this->Title = path.filename().string();
        this->Artist = "Unknown artist";
        this->Artists.push_back("Unknown Artist");
        this->Album = folderName;
        this->Genre = "Unknown genre";
        this->Year = "Unknown year";
        this->Length = "Unknown length";
        this->FileFormat = path.extension().c_str();
    }
    if(!f.isNull() && f.audioProperties()) {
        TagLib::AudioProperties *properties = f.audioProperties();
        this->Length = properties->length();
    } else {
        this->Length = "Unknown length";
    }

    this->Path = path;
    try {
        this->LastModified = std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(path).time_since_epoch()).count();
    } catch (std::filesystem::filesystem_error &e) {
        pluginLog(1, "Error while getting last modified time of file: " + path.string() + " Exception: " + e.what());
        this->LastModified = 0;
    }
    this->Cover = new CoverImage(path, deadbeef->conf_get_int(ML_ICON_SIZE, 32));
}

MediaFile::MediaFile() {
}

MediaFile::~MediaFile() {
}