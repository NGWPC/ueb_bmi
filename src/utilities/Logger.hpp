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

enum class LoggingModule {
	NGEN = 0,
	NOAHOWP, 
	SNOW17, 
	UEB, 
	CFE, 
	SACSMA, 
	LASAM, 
	SMP, 
	SFT, 
	TROUTE, 
	SCHISM, 
	SFINCS, 
	GC2D, 
	TOPOFLOW,
	MODULE_COUNT
};

/**
* Logger Class Used to Output Details of Current Application Flow
*/
class Logger {
  public:
	static std::shared_ptr<Logger> GetInstance();
	static std::string getLogLevelString(LogLevel level);
	void SetLogPreferences(LogLevel level);
	void Log(std::string message, LogLevel messageLevel);
	static LogLevel GetLogLevel(const std::string& logLevel);
	std::string createTimestamp();
	void setup_logger(void);
	std::string getLogFilePath();

  private:
	LogLevel logLevel;
	std::fstream logFile;
	static std::shared_ptr<Logger> loggerInstance;
	std::string logFilePath;
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
