#include "wavreader.hpp"

#include <wavfile.h>
#include <attachedpictureframe.h>

WavReader::WavReader() {
}

Glib::RefPtr<Gdk::Pixbuf> WavReader::getImage(std::filesystem::path path, int size) {
    TagLib::RIFF::WAV::File file(path.c_str());

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

TagLib::ByteVector* WavReader::getData(std::filesystem::path path) {
    TagLib::RIFF::WAV::File file(path.c_str());

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

WavReader::~WavReader() {
}