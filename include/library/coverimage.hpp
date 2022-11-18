#pragma once

#include <gtkmm.h>
#include <filesystem>

#include <iostream>
#include <tbytevector.h>

class CoverImage {
public:
    CoverImage(std::filesystem::path path, int pixBufSize);
    CoverImage();
    ~CoverImage();

    void regeneratePixbuf(int pixBufSize);

    std::vector<char> data;
    Glib::RefPtr<Gdk::Pixbuf> CoverPixbuf;
    std::string hash;

    template<class Archive>
    void serialize(Archive &a, const unsigned version){
        a & data & hash;
    }
};