#pragma once
#include <filesystem>
#include "Logger.h"
#include "TaskQueue.h"

class FileScanner {
public:
    static void scanAndEnqueue(const std::filesystem::path& sourceRoot,
                               const std::filesystem::path& destinationRoot,
                               TaskQueue& queue,
                               Logger& logger);
};