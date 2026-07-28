#pragma once

#include <string>

enum class LogLevel { Info, Warn, Error };

class Logger {
public:
    // Opens the log file for appending. Call once at startup, before any log() call.
    static void init(const std::string& filePath);

    static void info(const std::string& message)  { log(LogLevel::Info, message); }
    static void warn(const std::string& message)  { log(LogLevel::Warn, message); }
    static void error(const std::string& message) { log(LogLevel::Error, message); }

private:
    static void log(LogLevel level, const std::string& message);
};
