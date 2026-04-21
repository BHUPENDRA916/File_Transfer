#pragma once
#include <fstream>
#include <mutex>
#include <string>

class Logger {
public:
    explicit Logger(const std::string& logFilePath);

    void info(const std::string& message);
    void success(const std::string& message);
    void retry(const std::string& message);
    void error(const std::string& message);

private:
    std::string now();
    void write(const std::string& level, const std::string& message);

    std::mutex mutex_;
    std::ofstream out_;
};