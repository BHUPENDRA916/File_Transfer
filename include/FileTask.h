#pragma once
#include <filesystem>
#include <string>

struct FileTask {
    std::string relativePath;
    std::filesystem::path sourcePath;
    std::filesystem::path destinationPath;
};