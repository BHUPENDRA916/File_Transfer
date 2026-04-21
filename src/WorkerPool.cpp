#include "WorkerPool.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

WorkerPool::WorkerPool(std::size_t threadCount,
                       TaskQueue& queue,
                       Logger& logger,
                       StateStore& stateStore)
    : threadCount_(threadCount),
      queue_(queue),
      logger_(logger),
      stateStore_(stateStore) {}

void WorkerPool::start() {
    for (std::size_t i = 0; i < threadCount_; ++i) {
        workers_.emplace_back(&WorkerPool::workerLoop, this);
    }
}

void WorkerPool::wait() {
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void WorkerPool::workerLoop() {
    FileTask task;
    while (queue_.pop(task)) {
        processTask(task);
    }
}

void WorkerPool::cleanupTemp(const fs::path& tempPath) {
    std::error_code ec;
    fs::remove(tempPath, ec);
}

bool WorkerPool::reconcileExisting(const FileTask& task) {
    std::error_code ec;
    bool srcExists = fs::exists(task.sourcePath, ec);
    ec.clear();
    bool dstExists = fs::exists(task.destinationPath, ec);
    ec.clear();

    if (dstExists && !srcExists) {
        stateStore_.markCompleted(task.relativePath);
        logger_.success("Recovered completed file: " + task.relativePath);
        return true;
    }

    if (srcExists && dstExists) {
        std::string srcHash = Checksum::sha256File(task.sourcePath);
        std::string dstHash = Checksum::sha256File(task.destinationPath);

        if (!srcHash.empty() && srcHash == dstHash) {
            std::error_code removeEc;
            fs::remove(task.sourcePath, removeEc);
            if (!removeEc) {
                stateStore_.markCompleted(task.relativePath);
                logger_.success("Validated existing destination and removed source: " + task.relativePath);
                return true;
            }
        }
    }

    return false;
}

void WorkerPool::processTask(const FileTask& task) {
    if (stateStore_.isCompleted(task.relativePath)) {
        logger_.info("Skipped already completed: " + task.relativePath);
        return;
    }

    if (reconcileExisting(task)) {
        return;
    }

    for (int attempt = 1; attempt <= MAX_RETRIES; ++attempt) {
        fs::path tempPath;
        std::string errorMessage;

        if (!copier_.copyToTemp(task.sourcePath, task.destinationPath, tempPath, errorMessage)) {
            logger_.retry("Copy failed for " + task.relativePath + " (attempt " +
                          std::to_string(attempt) + "): " + errorMessage);
            cleanupTemp(tempPath);
            continue;
        }

        std::string sourceHash = Checksum::sha256File(task.sourcePath);
        std::string tempHash = Checksum::sha256File(tempPath);

        if (sourceHash.empty() || tempHash.empty()) {
            logger_.retry("Checksum failed for " + task.relativePath +
                          " (attempt " + std::to_string(attempt) + ")");
            cleanupTemp(tempPath);
            continue;
        }

        if (sourceHash != tempHash) {
            logger_.retry("Checksum mismatch for " + task.relativePath +
                          " (attempt " + std::to_string(attempt) + ")");
            cleanupTemp(tempPath);
            continue;
        }

        std::error_code ec;
        fs::create_directories(task.destinationPath.parent_path(), ec);
        ec.clear();

        fs::remove(task.destinationPath, ec);
        ec.clear();

        fs::rename(tempPath, task.destinationPath, ec);
        if (ec) {
            logger_.retry("Rename failed for " + task.relativePath +
                          " (attempt " + std::to_string(attempt) + "): " + ec.message());
            cleanupTemp(tempPath);
            continue;
        }

        fs::remove(task.sourcePath, ec);
        if (ec) {
            logger_.retry("Source delete failed for " + task.relativePath +
                          " (will recover on rerun): " + ec.message());
            return;
        }

        stateStore_.markCompleted(task.relativePath);
        logger_.success("Completed: " + task.relativePath);
        return;
    }

    logger_.error("FAILED after retries: " + task.relativePath);
}