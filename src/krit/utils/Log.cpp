#include "krit/utils/Log.h"

#include <chrono>
#include <cstdio>
#include <math.h>
#include <unistd.h>

namespace krit {

static const char *abbreviations[LogLevelCount] = {"DBG", "INF", "WRN",
                                                   "ERR", "...", "!!!"};
static const char *logOpen[LogLevelCount] = {
    "\u001b[34m", "\u001b[36m", "\u001b[33m", "\u001b[31m", "", "\u001b[35;1m"};
static const char *reset = "\u001b[0m";
static const char *bold = "\u001b[1m";

static const auto start_time = std::chrono::high_resolution_clock::now();
static bool logToTty = isatty(fileno(stdout));

LogLevel Log::defaultLevel = LogLevel::Error;
std::unordered_map<size_t, LogLevel> Log::levelMap;
std::vector<LogSink> Log::logSinks{Log::consoleLog};

LogLevel Log::level(std::string_view area) {
    return level(std::hash<std::string_view>()(area));
}

LogLevel Log::level(size_t area) {
    auto it = levelMap.find(area);
    return it == levelMap.end() ? defaultLevel : it->second;
}

void Log::log(LogLevel level, std::string_view area, const char *fmt,
              va_list args) {
    if (Log::level(area) > level) {
        return;
    }
    for (size_t i = 0; i < logSinks.size(); ++i) {
        auto &sink = logSinks[i];
        if (i == logSinks.size() - 1) {
            sink(level, area, fmt, args);
        } else {
            va_list argsCopy;
            va_copy(argsCopy, args);
            sink(level, area, fmt, argsCopy);
            va_end(argsCopy);
        }
    }
}

void Log::consoleLog(LogLevel level, std::string_view area, const char *fmt,
                     va_list args) {
    double s = std::chrono::duration<double, std::milli>(
                   std::chrono::high_resolution_clock::now() - start_time)
                   .count() /
               1000;
    int m = s / 60;
    s = fmod(s, 60);
    int h = m / 60;
    m = fmod(m, 60);
    if (logToTty) {
        printf("%s", bold);
    }
    printf("%02i:%02i:%06.3f ", h, m, s);
    if (logToTty) {
        printf("%s", logOpen[level]);
    }

    printf("<%.*s> [%s]:", (int)area.size(), area.data(), abbreviations[level]);
    if (logToTty) {
        printf("%s%s", reset, logOpen[level]);
    }
    vprintf(fmt, args);
    if (logToTty) {
        printf("%s\n", reset);
    } else {
        puts("");
    }
    fflush(stdout);
}

}
