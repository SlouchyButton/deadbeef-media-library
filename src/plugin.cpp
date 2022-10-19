#include "plugin.hpp"
#include "controller.hpp"
#include "settings.hpp"

DB_functions_t* deadbeef = NULL;
static DB_misc_t plugin;

const char config_dialog[] =
"property \"Replace playlist\" checkbox " ML_DOUBLECLICK_REPLACE " 1;\n"
"property \"Log verbosity\" select[4] " ML_LOG_VERBOSITY " 0 Errors Warnings Info Everything;\n"
"property \"Full library reimport needed for following settings\" label REIMPORTWARN;\n"
"property \"Icon size\" entry " ML_ICON_SIZE " 128;\n"
"property \"Icon quality\" select[4] " ML_ICON_QUALITY " 2 Highest High Medium Low;\n"
"property \"Highest quality increases memory usage, but has best load time.\nMemory consumption improvement has diminishing returns for qualities lower than Medium.\" label ICONNOTES;\n";

extern "C" 
DB_plugin_t* ddb_misc_media_library_load(DB_functions_t* api) {
    deadbeef = api;

    plugin.plugin.type = DB_PLUGIN_MISC;
    plugin.plugin.api_vmajor = 1;
    plugin.plugin.api_vminor = 15;
    plugin.plugin.version_major = 0;
    plugin.plugin.version_minor = 3;
    plugin.plugin.id = "media-library";
    plugin.plugin.name = "Media Library";
    plugin.plugin.descr = "Media Library plugin";
    plugin.plugin.copyright = "";
    plugin.plugin.website = "https://github.com/SlouchyButton/deadbeef-media-library";
    plugin.plugin.connect = &Controller::pluginConnect;
    plugin.plugin.disconnect = &Controller::pluginDisconnect;
    plugin.plugin.configdialog = config_dialog;
    plugin.plugin.flags = DDB_PLUGIN_FLAG_LOGGING;

    return DB_PLUGIN(&plugin.plugin);
}

void pluginLog(int level, std::string message) {
    int verbosity = deadbeef->conf_get_int(ML_LOG_VERBOSITY, 0);

    if (level <= verbosity) {
        std::string logLevel;
        if (level == 0) {
            logLevel = "ERROR";
            level = DDB_LOG_LAYER_DEFAULT;
        } else if (level == 1) {
            logLevel = "WARNING";
            level = DDB_LOG_LAYER_INFO;
        } else if (level == 2) {
            logLevel = "INFO";
            level = DDB_LOG_LAYER_INFO;
        } else {
            logLevel = "";
            level = DDB_LOG_LAYER_INFO;
        }

        std::string logMessage = "[Media Library] " + logLevel + " - " + message + "\n";
        deadbeef->log_detailed (&plugin.plugin, DDB_LOG_LAYER_DEFAULT, logMessage.c_str());
    }
}