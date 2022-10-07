#include "mp3reader.hpp"

#include <mpegfile.h>
#include <attachedpictureframe.h>

MP3Reader::MP3Reader() {
}

Glib::RefPtr<Gdk::Pixbuf> MP3Reader::getImage(std::filesystem::path path, int size) {
    TagLib::MPEG::File file(path.c_str());

    Glib::RefPtr<Gdk::Pixbuf> image;
    if (file.isValid()) {
        if (file.hasID3v2Tag()) {
            TagLib::ID3v2::Tag* tag = file.ID3v2Tag();
            image = this->getImageFromTag(tag, size);
            if (image.get()) {
                return image;
            }
        }
    }
    return image;
}

TagLib::ByteVector* MP3Reader::getData(std::filesystem::path path) {
    TagLib::MPEG::File file(path.c_str());

    TagLib::ByteVector* data = nullptr;
    if (file.isValid()) {
        if (file.hasID3v2Tag()) {
            TagLib::ID3v2::Tag* tag = file.ID3v2Tag();
            data = this->getDataFromTag(tag);
            if (data) {
                return data;
            }
        }
    }
    return data;
}

MP3Reader::~MP3Reader() {
}