#pragma once
#include <atomic>
#include <filesystem>
#include <thread>
#include <vector>
#include "Checksum.h"
#include "FileCopier.h"
#include "Logger.h"
#include "StateStore.h"
#include "TaskQueue.h"

class WorkerPool {
public:
    WorkerPool(std::size_t threadCount,
               TaskQueue& queue,
               Logger& logger,
               StateStore& stateStore);

    void start();
    void wait();

private:
    void workerLoop();
    void processTask(const FileTask& task);
    bool reconcileExisting(const FileTask& task);
    void cleanupTemp(const std::filesystem::path& tempPath);

    std::size_t threadCount_;
    TaskQueue& queue_;
    Logger& logger_;
    StateStore& stateStore_;
    FileCopier copier_;
    std::vector<std::thread> workers_;
    std::atomic<int> activeWorkers_{0};
    static constexpr int MAX_RETRIES = 3;
};