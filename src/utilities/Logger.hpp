#ifndef UEB_LOGGER_HPP
#define UEB_LOGGER_HPP

#ifdef USE_EWTS
#include "ewts/module_constants.hpp"
#include "ewts/logger.hpp"
#include "ewts/log_levels.hpp"

#define LOG(...) ::ewts::GetLogger(::ewts::modules::EWTS_ID_UEB).Log(__VA_ARGS__)
#define GetLogLevel() ::ewts::GetLogger(::ewts::modules::EWTS_ID_UEB).GetLogLevel()
#define IsLoggingEnabled() ::ewts::GetLogger(::ewts::modules::EWTS_ID_UEB).IsLoggingEnabled()

using ewts::EwtsInit;
using ewts::LogLevel;

inline constexpr const char* UEB_MODULE_ID = ewts::modules::EWTS_ID_UEB;

#else
// Log all messages to stdout
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

inline constexpr const char* UEB_MODULE_ID = "UEB_BMI";

// Minimal LogLevel enum (match EWTS values if possible)
enum class LogLevel {
    NOTSET = 0,
    DEBUG = 10,
    INFO = 20,
    WARNING = 30,
    SEVERE = 40,
    FATAL = 50
};

inline bool IsLoggingEnabled() {
    return true;  // always enabled in fallback
}

inline LogLevel GetLogLevel() {
    return LogLevel::INFO;  // simple default
}

inline const char* level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::SEVERE:  return "ERROR";
        case LogLevel::FATAL:   return "FATAL";
        default:                return "LOG";
    }
}

inline void Log(LogLevel level, std::string_view message) {
    std::ostringstream oss;
    oss << UEB_MODULE_ID << " "
        << level_to_string(level) << " "
        << message << '\n';

    std::cout << oss.str() << std::flush;   
}

inline void Log(std::string_view message, LogLevel level) {
    Log(level, message);
}

inline void Log(LogLevel level, const char* fmt, ...) {
    if (!fmt) return;

    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);
    int len = std::vsnprintf(nullptr, 0, fmt, args_copy);
    va_end(args_copy);

    if (len < 0) {
        va_end(args);
        return;
    }

    std::string buffer(static_cast<std::size_t>(len) + 1, '\0');
    std::vsnprintf(buffer.data(), buffer.size(), fmt, args);
    va_end(args);

    buffer.resize(static_cast<std::size_t>(len));

    Log(level, buffer);
}

#define LOG(...) Log(__VA_ARGS__)
#endif

#endif /* UEB_LOGGER_HPP */
