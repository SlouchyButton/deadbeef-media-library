#include "oggreader.hpp"

#include <vorbisfile.h>
#include <attachedpictureframe.h>

OggReader::OggReader() {
}

Glib::RefPtr<Gdk::Pixbuf> OggReader::getImage(std::filesystem::path path, int size) {
    TagLib::Ogg::Vorbis::File file(path.c_str());

    Glib::RefPtr<Gdk::Pixbuf> image;
    if (file.isValid()) {
        if (file.tag()) {
            TagLib::Ogg::XiphComment* tag = file.tag();
            TagLib::List<TagLib::FLAC::Picture*> pictureList = tag->pictureList();

            for (const auto &picture : pictureList) {
                image = this->getImageFromData(picture->data(), size);
                if (image.get()) {
                    return image;
                }
            }
        }
    }
    return image;
}

TagLib::ByteVector* OggReader::getData(std::filesystem::path path) {
    TagLib::Ogg::Vorbis::File file(path.c_str());

    TagLib::ByteVector* data = new TagLib::ByteVector();
    if (file.isValid()) {
        if (file.tag()) {
            TagLib::Ogg::XiphComment* tag = file.tag();
            TagLib::List<TagLib::FLAC::Picture*> pictureList = tag->pictureList();
            for (const auto &picture : pictureList) {
                *data = picture->data();
                if (data->size()) {
                    return data;
                }
            }
        }
    }
    return nullptr;
}

OggReader::~OggReader() {
}