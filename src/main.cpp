#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>

#include "FileScanner.h"
#include "Logger.h"
#include "StateStore.h"
#include "TaskQueue.h"
#include "WorkerPool.h"

namespace fs = std::filesystem;

// 🔥 Function to remove empty directories
void removeEmptyDirectories(const fs::path& root) {
    std::vector<fs::path> dirs;

    // Collect all directories
    for (auto& entry : fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied)) {
        if (fs::is_directory(entry)) {
            dirs.push_back(entry.path());
        }
    }

    // Sort in reverse order (deepest first)
    std::sort(dirs.rbegin(), dirs.rend());

    // Remove empty directories
    for (const auto& dir : dirs) {
        std::error_code ec;
        if (fs::is_empty(dir, ec)) {
            fs::remove(dir, ec);
        }
    }

    // Finally try removing root if empty
    std::error_code ec;
    if (fs::is_empty(root, ec)) {
        fs::remove(root, ec);
    }
}

int main(int argc, char* argv[]) {
    fs::path sourceRoot = "/source_ssd";
    fs::path destinationRoot = "/destination_hdd";
    std::size_t threadCount = 4;

    if (argc >= 3) {
        sourceRoot = argv[1];
        destinationRoot = argv[2];
    }

    if (argc >= 4) {
        threadCount = static_cast<std::size_t>(std::stoul(argv[3]));
    } else {
        unsigned int hc = std::thread::hardware_concurrency();
        threadCount = (hc == 0 ? 4 : static_cast<std::size_t>(hc));
    }

    fs::create_directories("logs");
    fs::create_directories(destinationRoot);

    Logger logger("logs/transfer.log");
    StateStore stateStore("logs/completed.manifest");
    TaskQueue queue;

    logger.info("Source: " + sourceRoot.string());
    logger.info("Destination: " + destinationRoot.string());
    logger.info("Worker threads: " + std::to_string(threadCount));

    WorkerPool pool(threadCount, queue, logger, stateStore);
    pool.start();

    // 🔹 Scan files and push tasks
    FileScanner::scanAndEnqueue(sourceRoot, destinationRoot, queue, logger);

    queue.close();
    pool.wait();

    // 🔥 NEW: remove empty folders from source
    removeEmptyDirectories(sourceRoot);

    logger.success("All work finished.");
    return 0;
}