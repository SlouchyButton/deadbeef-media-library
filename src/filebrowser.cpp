#include "filebrowser.hpp"

#include <string.h>
#include <algorithm>
#include <sys/stat.h>

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

#include <iostream>

std::unordered_map<std::string, int> Filebrowser::VALID_EXTENSIONS = std::unordered_map<std::string, int>();

std::vector<std::filesystem::directory_entry> Filebrowser::getFileList(std::filesystem::path path, bool sort, bool showHiddenFiles) {
    std::vector<std::filesystem::directory_entry> files = {};
    std::string entryLowercase;
    for(const auto &entry : std::filesystem::directory_iterator(path)) {
        struct stat buffer; 
        // Check permissions and errors
        if (stat(entry.path().c_str(), &buffer) != 0 || access(entry.path().c_str(), R_OK)) {
            continue;
        }

        // Hidden files
        if (!showHiddenFiles && entry.path().filename().string()[0] == '.') {
            continue;
        }

        // Add every directory, we can remove them later
        if (entry.is_directory()) {
            files.push_back(entry);
            continue;
        }

        // Check file extension
        if (Filebrowser::VALID_EXTENSIONS.find(entry.path().extension()) != Filebrowser::VALID_EXTENSIONS.end()) {
            files.push_back(entry);
        }
    }

    if (sort) {
        std::sort(files.begin(), files.end(), 
            [](const std::filesystem::directory_entry &s1, const std::filesystem::directory_entry &s2) -> bool {
                //compare date modified
                struct stat buffer1, buffer2;
                stat(s1.path().c_str(), &buffer1);
                stat(s2.path().c_str(), &buffer2);
                return buffer1.st_mtime > buffer2.st_mtime;
                //return strcasecmp(s1.path().c_str(), s2.path().c_str()) < 0 ? true : false;
            });
    }

    return files;
}

std::vector<std::filesystem::path> Filebrowser::getDirectoryList(std::filesystem::path path, bool sort, bool showHiddenFiles, bool includeItself) {
    std::vector<std::filesystem::path> directories = {};
    std::string entryLowercase;

    if (includeItself) {
        directories.push_back(path);
    }

    for(const auto &entry : std::filesystem::directory_iterator(path)) {
        struct stat buffer; 
        // Check permissions and errors
        if (stat(entry.path().c_str(), &buffer) != 0 || access(entry.path().c_str(), R_OK)) {
            continue;
        }

        // Hidden files
        if (!showHiddenFiles && entry.path().filename().string()[0] == '.') {
            continue;
        }

        // Add every directory, we can remove them later
        if (entry.is_directory()) {
            directories.push_back(entry.path());
        }
    }

    if (sort) {
        std::sort(directories.begin(), directories.end(), 
            [](const std::filesystem::path &s1, const std::filesystem::path &s2) -> bool {
                //compare date modified
                struct stat buffer1, buffer2;
                stat(s1.c_str(), &buffer1);
                stat(s2.c_str(), &buffer2);
                return buffer1.st_mtime > buffer2.st_mtime;
                //return strcasecmp(s1.path().c_str(), s2.path().c_str()) < 0 ? true : false;
            });
    }

    return directories;
}

bool Filebrowser::hasFile(std::filesystem::path directory, std::vector<std::string> acceptedFiles, std::string* outputFile) {
    auto path = directory.string();
    for (auto &file : acceptedFiles) {
        if (std::filesystem::exists(path + '/' + file)) {
            *outputFile = std::string(directory.string() + std::string("/") + file);
            return true;
        }
    }
    return false;
}