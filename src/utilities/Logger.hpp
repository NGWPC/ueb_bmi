#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <stdlib.h>
#include <string>
#include <iostream>
#include <memory>
#include <sstream>
#include <fstream>
#include <ctime>

enum class LogLevel {
	NONE = 0,
	DEBUG = 1,
	INFO = 2,
	ERROR = 3,
	WARN = 4,
	FATAL = 5,
};

/**
* Logger Class Used to Output Details of Current Application Flow
*/
class Logger {
  public:
	static std::shared_ptr<Logger> GetInstance();

	void        setup_logger(void);
    bool        CheckLogLevelEv(void);
    std::string CreateDateString(void);
	std::string CreateTimestamp(bool appendMS=true, bool iso=true);
    bool        CreateDirectory(const std::string& path);
    std::string ConvertLogLevelToString(LogLevel level);
 	LogLevel    ConvertStringToLogLevel(const std::string& logLevel);
    bool        DirectoryExists(const std::string& path);
    std::string GetLogFilePath(void);
    LogLevel    GetLogLevel(void);
	void        Log(std::string message, LogLevel messageLevel=LogLevel::INFO);
    bool        LogFileReady(void);
	void        SetLogPreferences(LogLevel level=LogLevel::INFO);
    std::string TrimString(const std::string& str);

  private:
    bool         envLogLevelLogged = false;
    std::fstream logFile;
    std::string  logFilePath = "";
    LogLevel     logLevel = LogLevel::ERROR;
    std::string  moduleName;

    static std::shared_ptr<Logger> loggerInstance;
    
  #if 0
    std::fstream& _out_stream;

    //Constructor: User provides custom output stream, or uses default (std::cout).
    public: Logger(std::fstream& stream = std::fstream): _out_stream(stream) {} 

    //Implicit conversion to std::ostream
    operator std::ostream() {
        return _out_stream;
    } 

    //Templated operator>> that uses the std::ostream: Everything that has defined 
    //an operator<< for the std::ostream (Everithing "printable" with std::cout 
    //and its colleages) can use this function.    
    template<typename T> 
    Logger& operator<< (const T& data) 
    {
        _out_stream << data;
    }
#endif
};



#endif
