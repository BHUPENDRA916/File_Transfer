#include "FileCopier.h"
#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

bool FileCopier::copyToTemp(const fs::path& source,
                            const fs::path& destinationFinal,
                            fs::path& tempPath,
                            std::string& errorMessage) {
    std::error_code ec;
    fs::create_directories(destinationFinal.parent_path(), ec);
    if (ec) {
        errorMessage = "Failed to create destination directory: " + ec.message();
        return false;
    }

    tempPath = destinationFinal;
    tempPath += ".part";

    fs::remove(tempPath, ec);
    ec.clear();

    std::ifstream in(source, std::ios::binary);
    if (!in.is_open()) {
        errorMessage = "Failed to open source file";
        return false;
    }

    std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        errorMessage = "Failed to open temp file for writing";
        return false;
    }

    const std::size_t BUFFER_SIZE = 8 * 1024 * 1024; // 8 MB
    std::vector<char> buffer(BUFFER_SIZE);

    while (in) {
        in.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        std::streamsize got = in.gcount();

        if (got > 0) {
            out.write(buffer.data(), got);
            if (!out) {
                errorMessage = "Write failure";
                out.close();
                fs::remove(tempPath, ec);
                return false;
            }
        }
    }

    if (in.bad()) {
        errorMessage = "Read failure";
        out.close();
        fs::remove(tempPath, ec);
        return false;
    }

    out.flush();
    if (!out) {
        errorMessage = "Flush failure";
        out.close();
        fs::remove(tempPath, ec);
        return false;
    }

    return true;
}