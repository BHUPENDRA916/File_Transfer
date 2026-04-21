#pragma once
#include <filesystem>
#include <string>

class Checksum {
public:
    static std::string sha256File(const std::filesystem::path& filePath);
};