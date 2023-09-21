#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>

void Logger::log(level_e level, const std::string& message) {
    switch (level) {
        case level_e::Info:
            std::cout << "INFO " << currentDateTime() << " - " << message << std::flush;
            break;
        case level_e::Warn:
            std::cout << "WARN " << currentDateTime() << " - " << message << std::flush;
            break;
        case level_e::Error:
            std::cout << "ERROR " << currentDateTime() << " - " << message << std::flush;
            break;
    }
}

std::string Logger::currentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto inTime = std::chrono::system_clock::to_time_t(now);

    const auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count() % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&inTime), "%d-%m-%Y %H:%M:%S") << '.' << std::setfill('0')
       << std::setw(3) << ms;

    return ss.str();
}