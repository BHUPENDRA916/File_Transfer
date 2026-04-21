#include "FileScanner.h"
#include <filesystem>

namespace fs = std::filesystem;

void FileScanner::scanAndEnqueue(const fs::path& sourceRoot,
                                 const fs::path& destinationRoot,
                                 TaskQueue& queue,
                                 Logger& logger) {
    std::error_code ec;

    if (!fs::exists(sourceRoot, ec)) {
        logger.error("Source root does not exist: " + sourceRoot.string());
        return;
    }

    fs::recursive_directory_iterator it(
        sourceRoot,
        fs::directory_options::skip_permission_denied,
        ec
    );

    fs::recursive_directory_iterator end;

    for (; it != end; it.increment(ec)) {
        if (ec) {
            logger.retry("Scan issue: " + ec.message());
            ec.clear();
            continue;
        }

        if (!it->is_regular_file(ec) || ec) {
            ec.clear();
            continue;
        }

        fs::path srcPath = it->path();
        fs::path relPath = fs::relative(srcPath, sourceRoot, ec);
        if (ec) {
            logger.retry("Could not build relative path for: " + srcPath.string());
            ec.clear();
            continue;
        }

        FileTask task;
        task.relativePath = relPath.string();
        task.sourcePath = srcPath;
        task.destinationPath = destinationRoot / relPath;

        queue.push(std::move(task));
    }

    logger.info("Scanning finished.");
}