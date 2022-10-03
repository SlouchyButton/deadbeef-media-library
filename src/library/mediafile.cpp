#include <fileref.h>
#include <tag.h>
#include <tpropertymap.h>

#include "mediafile.hpp"
#include "utils.hpp"
#include "plugin.hpp"
#include "settings.hpp"
#include "covermediafile.hpp"

MediaFile::MediaFile(std::filesystem::path path) {
    TagLib::FileRef f(path.c_str());

    if(!f.isNull() && f.tag()) {
        TagLib::Tag *tag = f.tag();
        this->Title = tag->title().isEmpty()? path.filename().string() : tag->title().toCString(true);
        this->Artist = tag->artist().isEmpty()? "Unknown artist" : tag->artist().toCString(true);
        this->Album = tag->album().isEmpty()? "Unknown album" : tag->album().toCString(true);
        this->Genre = tag->genre().isEmpty()? "Unkown genre" : tag->genre().toCString(true);
        this->Year = tag->year() == 0? "Unkown year" : std::to_string(tag->year());
        this->FileFormat = path.extension().c_str();
        for (auto &property : tag->properties()) {
            this->MetaData[property.first.toCString(true)] = property.second.front().toCString(true);
        }
    } else {
        this->Title = path.filename().string();
        this->Artist = "Unknown artist";
        this->Album = "Unknown album";
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
    
    Cache::Covers::CoverMediaFile coverMediaFile;
    this->CoverPixbuf = coverMediaFile.getIcon(this, deadbeef->conf_get_int(ML_ICON_SIZE, 32));
    this->coverFound = coverMediaFile.lastIconFound();
}

MediaFile::MediaFile() {
}

MediaFile::~MediaFile() {
}