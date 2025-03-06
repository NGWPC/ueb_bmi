#include "Logger.hpp"
#include <cstring>
#include <string>
#include <chrono>
#include <thread>
#include <cassert>

std::shared_ptr<Logger> Logger::loggerInstance;

using namespace std;

std::string module_name[static_cast<int>(LoggingModule::MODULE_COUNT)] 
{
	"NGEN    ",
	"NOAHOWP ", 
	"SNOW17  ", 
	"UEB     ", 
	"CFE     ", 
	"SACSMA  ", 
	"LASAM   ", 
	"SMP     ", 
	"SFT     ", 
	"TROUTE  ", 
	"SCHISM  ", 
	"SFINCS  ", 
	"GC2D    ", 
	"TOPOFLOW",
};

/**
* Configure Logger Preferences
* @param logFile
* @param level: LogLevel::ERROR by Default
* @param output: LogOutput::CONSOLE by Default
* @return void
*/
void Logger::SetLogPreferences(LogLevel level) {
	std::stringstream ss("");

	// get the log level for ueb_bmi module
	ss << getenv("ueb_bmi_ll");
 	LogLevel logLevel;
	if (ss.str() != "")
		logLevel = Logger::GetLogLevel(ss.str());
	else
		logLevel = level;

	std::cout << "UEB-BMI Log Level is set at: " << Logger::getLogLevelString(logLevel) << std::endl;
    
	// get the log file path
	ss.str("");
	ss << getenv("NGEN_LOG_FILE_PATH");
	logFilePath = ss.str();
	ss.str("");

	logFile.open(logFilePath, std::ios::app);
	if (!logFile.good()) {
		std::cerr << "Warning: Can't Open shared Log File referenced from NGEN_LOG_FILE_PATH env. variable for UEB-BMI module" << std::endl;
    	// create a local log file for UEB module instead
		std::string fwd_slash = "/";
    	std::string logFileName = "ueb_log.txt";
    	std::string logFileDir = "./run-logs/ngen_" + Logger::createTimestamp() + fwd_slash;
   		int status;
		std::string mkdir_cmd = "mkdir -p " + logFileDir;
		const char *cstr = mkdir_cmd.c_str();
   		status = system(cstr);
   		if (status == -1)
   		   std::cerr << "Error(" << (errno) << ") creating log file directory for UEB-BMI module: " << logFileDir << std::endl;
   		else
   		   std::cout << "Log directory for UEB-BMI module: " << logFileDir <<std::endl;
		// create a local log file for UEB module
		logFilePath = logFileDir+logFileName;
		logFile.open(logFilePath, ios::out | ios::app);
		if (!logFile.good()) {
			std::cerr << "Can't Open local directory Log File for UEB-BMI module:" << logFilePath <<std::endl;			
		}
		else {
			std::cout << "UEB-BMI module is logging instead into: " << logFilePath << std::endl;
		}
			
	}
	else {
		std::cout << "Log File Path for UEB-BMI module:" << logFilePath << std::endl;
	}

}

/**
* Convert LogLevel to String Representation of Log Level 
* @param logLevel : LogLevel
* @return String log level
*/
std::string Logger::getLogLevelString(LogLevel level) {
	std::string logType;
	//Set Log Level Name
	switch (level) {
		case LogLevel::FATAL:
			logType = "FATAL ";
			break;
		case LogLevel::DEBUG:
			logType = "DEBUG ";
			break;
		case LogLevel::INFO:
			logType = "INFO  ";
			break;
		case LogLevel::WARN:
			logType = "WARN  ";
			break;
		case LogLevel::ERROR:
			logType = "ERROR ";
			break;
		default:
			logType = "NONE  ";
			break;
	}
	return logType;
}

/**
* Get Single Logger Instance or Create new Object if Not Created
* @return std::shared_ptr<Logger>
*/
std::shared_ptr<Logger> Logger::GetInstance() {
	if (loggerInstance == nullptr) {
		loggerInstance = std::shared_ptr<Logger>(new Logger());
	}

	return loggerInstance;
}

/**
* Log given message with defined parameters and generate message to pass on Console or File
* @param message: Log Message
* @param messageLevel: Log Level, LogLevel::INFO by default
*/
void Logger::Log(std::string message, LogLevel messageLevel) {
	LoggingModule module=LoggingModule::UEB;

	// don't log if messageLevel < logLevel 
	if (messageLevel >= logLevel) {
		std::string logType;
		logType = Logger::getLogLevelString(messageLevel);

		std::string final_message;
		std::string mod_name;
		mod_name = module_name[static_cast<int>(module)];
		std::string separator = " ";
		// log the message while handling multiline cases
		final_message = createTimestamp() + separator + mod_name + separator + logType + message;
		if (!logFile.bad()) {
			logFile << final_message;
			std::cout << final_message;
			logFile.flush();
		}

	}
}

/**
* Convert String Representation of Log Level to LogLevel Type
* @param logLevel : String log level
* @return LogLevel
*/
LogLevel Logger::GetLogLevel(const std::string& logLevel) {
	if (logLevel == "DEBUG") {
		return LogLevel::DEBUG;
	}
	else if (logLevel == "INFO") {
		return LogLevel::INFO;
	}
	else if (logLevel == "WARN") {
		return LogLevel::ERROR;
	}
	else if (logLevel == "ERROR") {
		return LogLevel::ERROR;
	}
	else if (logLevel == "FATAL") {
		return LogLevel::ERROR;
	}

	return LogLevel::NONE;
}

using std::chrono::system_clock;

std::string Logger::createTimestamp() {
    std::chrono::_V2::system_clock::time_point currentTime = std::chrono::system_clock::now();
    char buffer1[120];
    char buffer2[120];
    std::stringstream ss;

    long transformed = currentTime.time_since_epoch().count() / 1000000;
    
    long millis = transformed % 1000;
    
    std::time_t tt;
    tt = system_clock::to_time_t ( currentTime );
    tm *timeinfo = gmtime (&tt);
    strftime (buffer1,100,"%FT%H:%M:%S",timeinfo);
    sprintf(buffer2, ":%03d", (int)millis);
	ss << buffer1 << buffer2;
    
    return ss.str();
}

void Logger::setup_logger(void) {
	std::stringstream std_ss;

	std_ss.str("");

    // One time log preferences
    (Logger::GetInstance())->SetLogPreferences(LogLevel::INFO);
}

std::string Logger::getLogFilePath() {
	return logFilePath;
}