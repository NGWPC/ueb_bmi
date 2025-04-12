#include "Logger.hpp"
#include <iostream>
#include <sstream>
#include <fstream>      // For file handling
#include <cstdlib>      // For getenv()
#include <cstring>
#include <string>       // For std::string
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <iomanip>
#include <chrono>
#include <sys/wait.h>
#include <sys/stat.h>
#include <thread>
#include <cassert>


const std::string MODULE_NAME      = "UEB_BMI ";          // Must be 8 characters for log entry alignment puroses.
const std::string LOG_PATH_DEFAULT = "./run-logs/ngen_";  // Default log directory string. Date to be appeneded if used
const std::string LOG_FILE_NAME    = "ueb_bmi";           // Log file name
const std::string LOG_FILE_EXT     = ".log";              // Log file name extension
const std::string DS               = "/";                 // Directory separator

std::shared_ptr<Logger> Logger::loggerInstance;

using namespace std;

// String to LogLevel map
static const std::unordered_map<std::string, LogLevel> logLevelMap = {
    {"NONE", LogLevel::NONE}, {"0", LogLevel::NONE},
    {"DEBUG", LogLevel::DEBUG}, {"1", LogLevel::DEBUG},
    {"INFO", LogLevel::INFO}, {"2", LogLevel::INFO},
    {"ERROR", LogLevel::ERROR}, {"3", LogLevel::ERROR},
    {"WARN", LogLevel::WARN}, {"4", LogLevel::WARN},
    {"FATAL", LogLevel::FATAL}, {"5", LogLevel::FATAL},
};

// Reverse map: LogLevel to String
static const std::unordered_map<LogLevel, std::string> logLevelToStringMap = {
    {LogLevel::NONE,  "NONE "},
    {LogLevel::DEBUG, "DEBUG"},
    {LogLevel::INFO,  "INFO "},
    {LogLevel::ERROR, "ERROR"},
    {LogLevel::WARN,  "WARN "},
    {LogLevel::FATAL, "FATAL"},
};

// Function to trim leading and trailing spaces
std::string Logger::TrimString(const std::string& str) {
    // Trim leading spaces
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) {
        return "";  // No non-whitespace characters
    }

    // Trim trailing spaces
    size_t last = str.find_last_not_of(" \t\n\r\f\v");

    // Return the trimmed string
    return str.substr(first, last - first + 1);
}

LogLevel Logger::ConvertStringToLogLevel(const std::string& logLevel) {
    // Convert string to LogLevel (supports both names and numbers)
   auto it = logLevelMap.find(logLevel);
   if (it != logLevelMap.end()) {
       return it->second;  // Found valid named or numeric log level
   }

   // Try parsing as an integer (for cases where an invalid numeric value is given)
   try {
       int levelNum = std::stoi(logLevel);
       if (levelNum >= 0 && levelNum <= 5) {
           return static_cast<LogLevel>(levelNum);
       }
   } catch (...) {
       // Ignore errors (e.g., if std::stoi fails for non-numeric input)
   }
   return LogLevel::NONE;
}

/**
* Convert LogLevel to String Representation of Log Level 
* @param logLevel : LogLevel
* @return String log level
*/
std::string Logger::ConvertLogLevelToString(LogLevel level) {
    auto it = logLevelToStringMap.find(level);
    if (it != logLevelToStringMap.end()) {
        return it->second;  // Found valid named or numeric log level
    }
    return "NONE";
}

bool Logger::DirectoryExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false; // Cannot access path
    }
    return (info.st_mode & S_IFDIR) != 0;
}

/**
 * Create the directory checking both the call
 * to execute the command and the result of the command
 */
bool Logger::CreateDirectory(const std::string& path) {

    if (!DirectoryExists(path)) {
        std::string mkdir_cmd = "mkdir -p " + path;
        int status = system(mkdir_cmd.c_str());

        if (status == -1) {
            std::cerr << "system() failed to run mkdir.\n";
            return false;
        } else if (WIFEXITED(status)) {
            int exitCode = WEXITSTATUS(status);
            if (exitCode != 0) {
                std::cerr << "mkdir command failed with exit code: " << exitCode << "\n";
                return false;
            }
        } else {
            std::cerr << "mkdir terminated abnormally.\n";
            return false;
        }
    }
    return true;
}

/**
 * Open log file and return open status. If already open,
 * ensure the write pointer is at the end of the file. 
 *
 * return bool true if open and good, false otherwise
 */
bool Logger::LogFileReady(void) {

    if (logFile.is_open() && logFile.good()) {
        logFile.seekp(0, std::ios::end); // Ensure write pointer is at the actual file end
        return true;
    }
    else {
        // Attempt to open shared file identified in the environment variable
        if (!envVarLogFilePath.empty()) {
            logFile.open(envVarLogFilePath, ios::out | ios::app); // This will silently fail if already open.
            if (logFile.good()) {
                logFilePath = envVarLogFilePath;
                return true;
            }
        }

        // Unable to upen shared file so attempt to open alternate log file

        // Determine if an alternate file directory exists with today's timestamp
        std::string todaysDate = CreateDateString();
        if (!logFilePath.empty() && (logFilePath.find(todaysDate) != std::string::npos)) {
            logFile.open(logFilePath, ios::out | ios::app); // This will silently fail if already open.
            if (logFile.good()) {
                return true;
            }
        }

        // Alternate log file doesn't exist
        std::string altDir = "./run-logs/ngen_" + todaysDate;
        if (CreateDirectory(altDir)) {
            logFilePath = altDir + "/ueb_bmi" + "_" + Logger::CreateTimestamp(false) + ".log";
            logFile.open(logFilePath, ios::out | ios::app); // This will silently fail if already open.
            if (logFile.good()) {
                return true;
            }
        }
    }
    return false;
}


/**
 * Check the log level envinroment variable and update it if it changed.
 * @return true if environment variable exists otherwise false. 
 *
*/
bool Logger::CheckLogLevelEv(void) {
    const char* envLogLevel = std::getenv("UEB_BMI_LOGLEVEL");
    if (envLogLevel != nullptr && envLogLevel[0] != '\0') {
        LogLevel envll = ConvertStringToLogLevel(envLogLevel);
        if (envll != logLevel) {
            logLevel = envll;
            std::string llMsg = "INFO: UEB_BMI log level set to UEB_BMI_LOGLEVEL (" + TrimString(ConvertLogLevelToString(logLevel)) + ")\n";
            // This is an INFO message that always should be in the log but the
            // logLevel could be different than INFO. herefore use logLevel to 
            // ensure the message is recorded in the log
            Log(llMsg, logLevel);
        }
        return true;
    }
    return false;
}

/**
* Configure Logger Preferences
* @param logFile
* @param level: LogLevel::INFO by Default
* @return void
*/
void Logger::SetLogPreferences(LogLevel level) {

    // Make sure the module name used for logging is all uppercase and 8 characters wide.
    moduleName = MODULE_NAME;
    std::string upperName = moduleName.substr(0, 8);  // Truncate to 8 chars max
    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);

    std::ostringstream oss;
    oss << std::left << std::setw(8) << std::setfill(' ') << upperName;
    moduleName = oss.str();
    
    // get the log file path from environment (written by ngen Logger)
    const char* envVar = std::getenv("NGEN_LOG_FILE_PATH");
    if (envVar != nullptr && envVar[0] != '\0') {
        envVarLogFilePath = envVar;
    }

    // Open log file. If path not found, it will try an alternate file. If neither
    // works false returned and logs will be written to stdout.
    if (LogFileReady()) {
        std::string msg = "INFO: UEB_BMI Logging started. Log File Path: " + logFilePath + "\n";
        // This is an INFO message that always should be in the log but the
        // logLevel could be different than INFO. Therefore use logLevel to 
        // ensure the message is recorded in the log
        Log(msg, logLevel);
    }
    else {
        std::cerr << "Unable to open primary or altnerante log File." << std::endl;			
        std::cerr << "Log entries will be written to stdout." << std::endl;
    }

    std::string llMsg = "INFO: UEB_BMI default log level is " + TrimString(ConvertLogLevelToString(logLevel)) + "\n";
    // This is an INFO message that always should be in the log but the
    // logLevel could be different than INFO. Therefore use logLevel to 
    // ensure the message is recorded in the log
    Log(llMsg, logLevel);

    // Set to environment log level variable if found
    if (!CheckLogLevelEv()) logLevel = level;
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
    // Check for change in log level environment variable
    (void) CheckLogLevelEv();

    // don't log if messageLevel < logLevel 
    if (messageLevel >= logLevel) {
        std::string   logType   = ConvertLogLevelToString(messageLevel);
        std::string   logPrefix = CreateTimestamp() + " " + moduleName + " " + logType;

        // Log message, creating individual entries for a multi-line message
        std::istringstream logMsg(message);
        std::string line;
        if (LogFileReady()) {
            while (std::getline(logMsg, line)) {
                logFile << logPrefix + " " + line << std::endl;
            }
            logFile.flush();
        }
        else {
            // Log file not found. Write to stdout.
            while (std::getline(logMsg, line)) {
                cout << logPrefix + " " + line << std::endl;
            }
            cout << std::flush;
        }
    }
}

std::string Logger::CreateDateString(void) {
    std::time_t tt = std::time(0);
    std::tm* timeinfo = std::gmtime(&tt); // Use std::localtime(&tt) if you want local time

    char buffer[11]; // Enough for "YYYY-MM-DD" + null terminator
    std::strftime(buffer, sizeof(buffer), "%F", timeinfo); // %F == %Y-%m-%d

    std::stringstream ss;
    ss << buffer;

    return ss.str();
}

std::string Logger::CreateTimestamp(bool appendMS) {
    using namespace std::chrono;

    // Get current time point
    auto now = system_clock::now();
    auto now_time_t = system_clock::to_time_t(now);

    // Get milliseconds
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // Convert to UTC time
    std::tm utc_tm;
    gmtime_r(&now_time_t, &utc_tm);

    if (appendMS) {
        // Format date/time with strftime
        char buffer[32];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &utc_tm);

        // Combine with milliseconds
        std::ostringstream oss;
        oss << buffer << '.' << std::setw(3) << std::setfill('0') << ms.count();

        return oss.str();
    }
    // Format ts for alt logfile name 
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y%m%dT%H%M%S", &utc_tm);
    
    return std::string(buffer);
}

void Logger::setup_logger(void) {
    // One time log preferences
    (Logger::GetInstance())->SetLogPreferences(LogLevel::INFO);
}

std::string Logger::GetLogFilePath() {
	return logFilePath;
}
