#include "covermediafile.hpp"

#include "readerfactory.hpp"
#include "filebrowser.hpp"
#include "plugin.hpp"

#include "iostream"

using namespace Cache::Covers;

CoverMediaFile::CoverMediaFile() {
}

Glib::RefPtr<Gdk::Pixbuf> CoverMediaFile::getIcon(MediaFile* mediaFile, uint size) {
    Glib::RefPtr<Gdk::Pixbuf> icon;
    static Glib::RefPtr<Gdk::Pixbuf> lastIcon;
    static std::string lastDirectory;

    std::string cache = deadbeef->get_system_dir(DDB_SYS_DIR_CACHE);
    cache += "/media-library/icons/media-files/" + std::to_string(size) + std::string("/");
    if (!std::filesystem::exists(cache)) {
        std::filesystem::create_directories(cache);
    }
    cache += std::to_string(std::hash<std::string>{}(mediaFile->Path))+".png";
    std::filesystem::path currentPath = cache;

    if (std::filesystem::exists(currentPath)) {
        icon = Gdk::Pixbuf::create_from_file(currentPath);
        status = true;
    } else {
        bool useCache = false;
        icon = ReaderFactory::getImage(mediaFile->Path, size, &useCache);
        if (useCache) {
            icon->save(currentPath, "png");
        }
        status = useCache;
    }
    return icon;
}

bool CoverMediaFile::lastIconFound() {
    return status;
}

CoverMediaFile::~CoverMediaFile() {
}
