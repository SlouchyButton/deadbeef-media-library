#pragma once 

#include <gtkmm.h>
#include <filesystem>
#include <thread>
#include <unordered_set>

class Benchmark {
public:
    Benchmark();
    ~Benchmark();
    void fileExtensionDataType();
private:
    std::thread* mBenchmarkThread;
    std::unordered_map<std::string, int> ext_un_map;
    std::map<std::string, int> ext_map;
    std::vector<std::string> ext_vec;
    std::unordered_set<std::string> ext_un_set;
    std::set<std::string> ext_set;

    void fileExtensionDataTypeThread();
    std::vector<std::filesystem::directory_entry> files;
    void getFiles(std::filesystem::path path, int type);
};
