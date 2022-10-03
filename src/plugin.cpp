#include "plugin.hpp"
#include "controller.hpp"
#include "settings.hpp"

DB_functions_t* deadbeef = NULL;
static DB_misc_t plugin;

const char config_dialog[] =
"property \"Icon size\" entry " ML_ICON_SIZE " 128;\n"
"property \"Replace playlist\" checkbox " ML_DOUBLECLICK_REPLACE " 1;\n";

extern "C" 
DB_plugin_t* ddb_misc_media_library_load(DB_functions_t* api) {
    deadbeef = api;

    plugin.plugin.type = DB_PLUGIN_MISC;
    plugin.plugin.api_vmajor = 1;
    plugin.plugin.api_vminor = 15;
    plugin.plugin.version_major = 0;
    plugin.plugin.version_minor = 1;
    plugin.plugin.id = "media-library";
    plugin.plugin.name = "Media Library";
    plugin.plugin.descr = "Media Library plugin";
    plugin.plugin.copyright = "GPL-3.0";
    plugin.plugin.website = "https://github.com/SlouchyButton/deadbeef-media-library";
    plugin.plugin.connect = &Controller::pluginConnect;
    plugin.plugin.disconnect = &Controller::pluginDisconnect;
    plugin.plugin.configdialog = config_dialog;
    plugin.plugin.flags = DDB_PLUGIN_FLAG_LOGGING;

    return DB_PLUGIN(&plugin.plugin);
}

void pluginLog(int level, std::string message) {
    std::string logMessage = "Media Library: " + message + "\n";
    deadbeef->log_detailed (&plugin.plugin, level, message.c_str());
}