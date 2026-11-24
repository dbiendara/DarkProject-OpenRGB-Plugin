#include "LogManager.h"

LogManager::LogManager() = default;
LogManager::~LogManager() = default;

LogManager* LogManager::get()
{
    static LogManager instance;
    return &instance;
}

void LogManager::LogEntry(const char* /*filename*/, int /*line*/, unsigned int /*level*/, const char* /*fmt*/, ...)
{
}
