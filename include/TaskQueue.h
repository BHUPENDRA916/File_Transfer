#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include "FileTask.h"

class TaskQueue {
public:
    void push(FileTask task) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(task));
        }
        cv_.notify_one();
    }

    bool pop(FileTask& task) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&] { return closed_ || !queue_.empty(); });

        if (queue_.empty()) {
            return false;
        }

        task = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        cv_.notify_all();
    }

private:
    std::queue<FileTask> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool closed_ = false;
};