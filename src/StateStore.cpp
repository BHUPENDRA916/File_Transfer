#include "StateStore.h"
#include <fstream>

StateStore::StateStore(const std::string& manifestPath)
    : manifestPath_(manifestPath) {
    load();
}

void StateStore::load() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ifstream in(manifestPath_);
    if (!in.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            completed_.insert(line);
        }
    }
}

bool StateStore::isCompleted(const std::string& relativePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    return completed_.find(relativePath) != completed_.end();
}

void StateStore::markCompleted(const std::string& relativePath) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto inserted = completed_.insert(relativePath);
    if (!inserted.second) {
        return;
    }

    std::ofstream out(manifestPath_, std::ios::app);
    if (out.is_open()) {
        out << relativePath << '\n';
        out.flush();
    }
}