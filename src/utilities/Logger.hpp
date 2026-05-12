#ifndef UEB_LOGGER_HPP
#define UEB_LOGGER_HPP

#include "ewts/module_constants.hpp"
#include "ewts/logger.hpp"
#include "ewts/log_levels.hpp"

#define LOG(...) ::ewts::GetLogger(::ewts::modules::EWTS_ID_UEB).Log(__VA_ARGS__)
#define GetLogLevel() ::ewts::GetLogger(::ewts::modules::EWTS_ID_UEB).GetLogLevel()
#define IsLoggingEnabled() ::ewts::GetLogger(::ewts::modules::EWTS_ID_UEB).IsLoggingEnabled()

using ewts::EwtsInit;
using ewts::LogLevel;

inline constexpr const char* UEB_MODULE_ID = ewts::modules::EWTS_ID_UEB;

#endif /* UEB_LOGGER_HPP */
