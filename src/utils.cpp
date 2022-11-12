#include "utils.hpp"

#include "deadbeef/deadbeef.h"

#include <flacpicture.h>
#include <flacfile.h>
#include <id3v2tag.h>
#include <id3v1tag.h>
#include <attachedpictureframe.h>
#include <mpegfile.h>
#include <wavfile.h>
#include <oggfile.h>
#include <vorbisfile.h>

#include <boost/algorithm/string.hpp>

#include "filebrowser.hpp"
#include "plugin.hpp"
#include "settings.hpp"

std::string Utils::escapeTooltip(std::string tooltip) {
    uint i = 0;
    std::string n = tooltip;
    // Memory leak? See Valgrind
    for (char const &c : n) {
        i++;
        if (c == '&') {
            n.insert(i, "amp;");
        }
    }
    return n;
}

Glib::RefPtr<Gdk::Pixbuf> Utils::getIconByName(const char* name, uint size, bool force) {
    Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
    Gtk::IconInfo icon = theme->lookup_icon(name, size);
    if (icon) {
        return icon.load_icon();
    } else if (force) {
        Glib::RefPtr<Gdk::Pixbuf> invalidIcon = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, size, size);
        invalidIcon->fill(0xFF00FFFF);
        return invalidIcon;
    } else {
        throw std::runtime_error("Icon " + std::string(name) + " not found");
    }
}

std::unordered_map<std::string, int> Utils::createValidExtensions() {
    std::unordered_map<std::string, int> extensions;

    // Array
    struct DB_decoder_s **decoders = deadbeef->plug_get_decoder_list();
    for (gint i = 0; decoders[i]; i++) {
        const char** exts = decoders[i]->exts;
        for (gint j = 0; exts[j]; j++) {
            // Does anybody use this ridicilously long extensions?
            GString *buf = g_string_sized_new(32);
            g_string_append_printf(buf, ".%s", exts[j]);
            extensions[buf->str] = 1;
        }
    }
    return extensions;
}

//Untested
std::vector<std::string> Utils::splitString(std::string str, std::string delimiter) {
    std::vector<std::string> result;
    
    std::string substring = str;
    std::size_t pos = str.find(delimiter);

    if (pos == std::string::npos) {
        result.push_back(substring);
        return result;
    }

    while (pos != std::string::npos) {
        substring = substring.substr(0, pos);
        result.push_back(substring);
        pos = substring.find(delimiter);
    }
    
    return result;
}