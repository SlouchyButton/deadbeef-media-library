#include "benchmark.hpp"

#include "filebrowser.hpp"
#include "utils.hpp"
#include "plugin.hpp"
#include <chrono>
#include <iostream>
#include <unordered_set>

Benchmark::Benchmark() {
    // Array
    struct DB_decoder_s **decoders = deadbeef->plug_get_decoder_list();
    for (gint i = 0; decoders[i]; i++) {
        const char** exts = decoders[i]->exts;
        for (gint j = 0; exts[j]; j++) {
            // Does anybody use this ridicilously long extensions?
            GString *buf = g_string_sized_new(32);
            g_string_append_printf(buf, ".%s", exts[j]);
            this->ext_un_map[buf->str] = 1;
            this->ext_set.insert(buf->str);
            this->ext_vec.push_back(buf->str);
            this->ext_un_set.insert(buf->str);
            this->ext_map[buf->str] = 1;
        }
    }
}

void Benchmark::fileExtensionDataType() {
    mBenchmarkThread = new std::thread(&Benchmark::fileExtensionDataTypeThread, this);
}

void Benchmark::fileExtensionDataTypeThread() {
    this->files = {};
    getFiles("/home/slouchy/Music", 0);
    std::cout << "Files: " << this->files.size() << std::endl;

    this->files = {};
    auto start = std::chrono::high_resolution_clock::now();
    getFiles("/home/slouchy/Music", 0);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Using map: " << duration.count() << " ms" << std::endl;

    this->files = {};
    start = std::chrono::high_resolution_clock::now();
    getFiles("/home/slouchy/Music", 1);
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Using unordered map: " << duration.count() << " ms" << std::endl;

    this->files = {};
    start = std::chrono::high_resolution_clock::now();
    getFiles("/home/slouchy/Music", 2);
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Using vector: " << duration.count() << " ms" << std::endl;

    this->files = {};
    start = std::chrono::high_resolution_clock::now();
    getFiles("/home/slouchy/Music", 3);
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Using set: " << duration.count() << " ms" << std::endl;

    this->files = {};
    start = std::chrono::high_resolution_clock::now();
    getFiles("/home/slouchy/Music", 4);
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Using unordered set (find): " << duration.count() << " ms" << std::endl;

    this->files = {};
    start = std::chrono::high_resolution_clock::now();
    getFiles("/home/slouchy/Music", 5);
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Using unordered set (contains): " << duration.count() << " ms" << std::endl;

}

void Benchmark::getFiles(std::filesystem::path path, int type) {
    std::string entryLowercase;
    for(const auto &entry : std::filesystem::directory_iterator(path)) {
        // Replace with non-POSIX?
        struct stat buffer; 
        // Check permissions and errors
        if (stat(entry.path().c_str(), &buffer) != 0 || access(entry.path().c_str(), R_OK)) {
            continue;
        }

        // Add every directory, we can remove them later
        if (entry.is_directory()) {
            getFiles(entry.path(), type);
        }

        bool test = false;
        // Check file extension
        if (type == 0) {
            if (ext_map.find(entry.path().extension()) != ext_map.end()) {
                files.push_back(entry);
            }
        } else if (type == 1) {
            if (ext_un_map.find(entry.path().extension()) != ext_un_map.end()) {
                files.push_back(entry);
            }
        } else if (type == 2) {
            bool validFile = false;
            for (auto &extension : ext_vec) {
                if (entry.path().extension() == extension) {
                    validFile = true;
                    break;
                }
            }
            if (validFile) {
                files.push_back(entry);
            }
        } else if (type == 3) {
            if (ext_set.find(entry.path().extension()) != ext_set.end()) {
                files.push_back(entry);
            }
        } else if (type == 4) {
            if (ext_un_set.find(entry.path().extension()) != ext_un_set.end()) {
                files.push_back(entry);
            }
        } else if (type == 5) {
            if (ext_un_set.contains(entry.path().extension())) {
                files.push_back(entry);
            }
        }
        
    }
}

Benchmark::~Benchmark() {
}