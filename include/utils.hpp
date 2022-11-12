#pragma once 

#include <gtkmm.h>
#include <filesystem>

class Utils {
public:
    /**
     * Replaces & with &amp; as defined by GTK.
     * 
     * @param tooltip String to be cleaned.
     * 
     * @return Edited string.
     */
    static std::string escapeTooltip(std::string tooltip);

    /**
     * Retrieves local theme's gtk icon by it's name in desired resolution.
     * 
     * @param name Gtk icon name, see icons in Glade or documentation
     * @param size Desired resolution
     * 
     * @return Pointer to the icon, if name is resolved, nullptr otherwise.
     */
    static Glib::RefPtr<Gdk::Pixbuf> getIconByName(const char* name, uint size, bool force = true);

    /**
     * Retrieves all supported extensions via deadbeef api
     * 
     * @return Vector of all supported extensions
     */
    static std::unordered_map<std::string, int> createValidExtensions();

    /**
     * Split string by delimiter. NOTE: Not tested.
     *
     * @return  std::vector    Vector of strings
     */
    static std::vector<std::string> splitString(std::string str, std::string delimiter);
private:
    Utils();
    ~Utils();
};
