#pragma once

#include <gtkmm.h>
#include <filesystem>

#include <iostream>
#include <tbytevector.h>

class CoverImage {
public:
    CoverImage(TagLib::ByteVector bytes, int pixBufSize);
    CoverImage(std::filesystem::path path, int pixBufSize);
    CoverImage(std::vector<char> bytes, int pixBufSize);
    CoverImage();
    ~CoverImage();

    void regeneratePixbuf(int pixBufSize);
    void generatePixbuf(TagLib::ByteVector bytes, int pixBufSize);

    std::vector<char> data;
    int size;
    Glib::RefPtr<Gdk::Pixbuf> CoverPixbuf;

    template<class Archive>
    void serialize(Archive &a, const unsigned version){
        a & data;
    }

private:
    void loadMetaData();
};