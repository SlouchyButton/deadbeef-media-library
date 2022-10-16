#include "coverimage.hpp"

#include "plugin.hpp"
#include "settings.hpp"
#include "readerfactory.hpp"
#include "utils.hpp"

#include <iostream>
#include <fstream>

CoverImage::CoverImage(TagLib::ByteVector tagLibData, int pixBufSize) {
    this->data = std::vector<char>(tagLibData.begin(), tagLibData.end());
    
    this->regeneratePixbuf(pixBufSize);
}

CoverImage::CoverImage(std::filesystem::path path, int pixBufSize) {
    TagLib::ByteVector* tagLibData = ReaderFactory::getImageData(path);



    if (tagLibData) {
        if (tagLibData->size() > 0) {
            Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create();
            loader->set_size(pixBufSize, pixBufSize);
            try {
                loader->write((const unsigned char*)tagLibData->data(), tagLibData->size());
                loader->close();
                this->CoverPixbuf = loader->get_pixbuf();
            } catch (Gdk::PixbufError &e) {
                pluginLog(1, ("Couldn't load cover from metadata: " + e.what()));
            }
        }
        int quality = deadbeef->conf_get_int(ML_ICON_QUALITY, 2);
        gchar* gData = new gchar();
        gsize gSize = 0;
        if (quality == 0) {
            this->CoverPixbuf->save_to_buffer(gData, gSize, "bmp");
        } else {
            std::vector<Glib::ustring> optionKeys = {"quality"};
            std::vector<Glib::ustring> optionValues;
            if (quality == 1) {
                optionValues = {"100"};
            } else if (quality == 2) {
                optionValues = {"75"};
            } else {
                optionValues = {"50"};
            }
            
            
            this->CoverPixbuf->save_to_buffer(gData, gSize, "jpeg", optionKeys, optionValues);
        }

        this->data = std::vector<char>(gData, gData + gSize);
        delete gData;
        delete tagLibData;
    } else {
        this->data = std::vector<char>();
        this->regeneratePixbuf(pixBufSize);
    }    
}

CoverImage::CoverImage(std::vector<char> data, int pixBufSize) {
    this->data = data;
    
    this->regeneratePixbuf(pixBufSize);
}

CoverImage::CoverImage() {
}

void CoverImage::generatePixbuf(TagLib::ByteVector tagLibData, int pixBufSize) {
    this->data = std::vector<char>(tagLibData.begin(), tagLibData.end());
    
    this->regeneratePixbuf(pixBufSize);
}

void CoverImage::regeneratePixbuf(int pixBufSize) {
    if (this->data.size() > 0) {
        Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create();
        loader->set_size(pixBufSize, pixBufSize);
        try {
            loader->write((const unsigned char*)this->data.data(), this->data.size());
            loader->close();
            this->CoverPixbuf = loader->get_pixbuf();
        } catch (Gdk::PixbufError &e) {
            pluginLog(1, ("Couldn't load cover from metadata: " + e.what()));
        }
    } else {
        try {
            this->CoverPixbuf = Utils::getIconByName("media-default-album", pixBufSize, false);
        } catch (std::exception &e) {
            this->CoverPixbuf = Utils::getIconByName("media-optical", pixBufSize);
        }
    }
}

CoverImage::~CoverImage() {
}