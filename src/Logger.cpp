#include "Logger.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

Logger::Logger(const std::string& logFilePath) {
    out_.open(logFilePath, std::ios::app);
}

std::string Logger::now() {
    using namespace std::chrono;
    auto t = system_clock::to_time_t(system_clock::now());
    std::tm tm{};

#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Logger::write(const std::string& level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string line = "[" + now() + "][" + level + "] " + message;

    std::cout << line << std::endl;
    if (out_.is_open()) {
        out_ << line << '\n';
        out_.flush();
    }
}

void Logger::info(const std::string& message) {
    write("INFO", message);
}

void Logger::success(const std::string& message) {
    write("SUCCESS", message);
}

void Logger::retry(const std::string& message) {
    write("RETRY", message);
}

void Logger::error(const std::string& message) {
    write("ERROR", message);
}