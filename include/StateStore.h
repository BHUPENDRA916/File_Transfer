#pragma once
#include <mutex>
#include <string>
#include <unordered_set>

class StateStore {
public:
    explicit StateStore(const std::string& manifestPath);

    bool isCompleted(const std::string& relativePath);
    void markCompleted(const std::string& relativePath);

private:
    void load();

    std::string manifestPath_;
    std::unordered_set<std::string> completed_;
    std::mutex mutex_;
};