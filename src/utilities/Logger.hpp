#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdlib.h>
#include <string>

enum class LogLevel {
    NONE  = 0,
    DEBUG = 1,
    INFO  = 2,
    ERROR = 3,
    WARN  = 4,
    FATAL = 5,
};

/**
 * Logger Class Used to Output Details of Current Application Flow
 */
class Logger {
  public:
    static std::shared_ptr<Logger> GetInstance();

    void setup_logger(void);
    bool CheckLogLevelEv(void);
    std::string CreateDateString(void);
    std::string CreateTimestamp(bool appendMS = true, bool iso = true);
    bool CreateDirectory(const std::string& path);
    std::string ConvertLogLevelToString(LogLevel level);
    LogLevel ConvertStringToLogLevel(const std::string& logLevel);
    bool DirectoryExists(const std::string& path);
    std::string GetLogFilePath(void);
    LogLevel GetLogLevel(void);
    void Log(std::string message, LogLevel messageLevel = LogLevel::INFO);
    bool LogFileReady(bool appendMode=true);
    void SetLogFilePath(void);
    void SetLogLevel(LogLevel level);
    void SetLogPreferences(LogLevel level = LogLevel::INFO);
    std::string TrimString(const std::string& str);

  private:
    bool envLogLevelLogged = false;
    std::fstream logFile;
    std::string logFilePath = "";
    LogLevel logLevel       = LogLevel::ERROR;
    std::string moduleName;

    static std::shared_ptr<Logger> loggerInstance;
};

#endif
