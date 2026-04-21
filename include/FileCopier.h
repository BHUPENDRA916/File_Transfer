#pragma once
#include <filesystem>
#include <string>

class FileCopier {
public:
    bool copyToTemp(const std::filesystem::path& source,
                    const std::filesystem::path& destinationFinal,
                    std::filesystem::path& tempPath,
                    std::string& errorMessage);
};