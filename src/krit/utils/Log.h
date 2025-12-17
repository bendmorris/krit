#ifndef KRIT_UTILS_LOG
#define KRIT_UTILS_LOG

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <functional>
#include <string_view>
#include <unistd.h>
#include <vector>

namespace krit {

struct cmp_str {
    bool operator()(char const *a, char const *b) const {
        return std::strcmp(a, b) < 0;
    }
};

#define LOG_DEBUG(...)                                                         \
    if (krit::Log::defaultLevel <= LogLevel::Debug) {                          \
        Log::debug("krit", __VA_ARGS__);                                       \
    }
#define LOG_INFO(...)                                                          \
    if (krit::Log::defaultLevel <= LogLevel::Info) {                           \
        Log::info("krit", __VA_ARGS__);                                        \
    }
#define LOG_WARN(...)                                                          \
    if (krit::Log::defaultLevel <= LogLevel::Warn) {                           \
        Log::warn("krit", __VA_ARGS__);                                        \
    }
#define LOG_ERROR(...)                                                         \
    if (krit::Log::defaultLevel <= LogLevel::Error) {                          \
        Log::error("krit", __VA_ARGS__);                                       \
    }
#define LOG_OUTPUT(...)                                                        \
    if (krit::Log::defaultLevel <= LogLevel::Output) {                         \
        Log::output("krit", __VA_ARGS__);                                      \
    }
#define LOG_FATAL(...)                                                         \
    if (krit::Log::defaultLevel <= LogLevel::Fatal) {                          \
        Log::fatal("krit", __VA_ARGS__);                                       \
    }

#define AREA_LOG_DEBUG(area, ...)                                              \
    if (krit::Log::defaultLevel < LogLevel::Debug ||                           \
        krit::Log::level(std::hash<std::string_view>()(area)) <=               \
            LogLevel::Debug) {                                                 \
        Log::debug(area, __VA_ARGS__);                                         \
    }
#define AREA_LOG_INFO(area, ...)                                               \
    if (krit::Log::defaultLevel < LogLevel::Info ||                            \
        krit::Log::level(std::hash<std::string_view>()(area)) <=               \
            LogLevel::Info) {                                                  \
        Log::info(area, __VA_ARGS__);                                          \
    }
#define AREA_LOG_WARN(area, ...)                                               \
    if (krit::Log::defaultLevel < LogLevel::Warn ||                            \
        krit::Log::level(std::hash<std::string_view>()(area)) <=               \
            LogLevel::Warn) {                                                  \
        Log::warn(area, __VA_ARGS__);                                          \
    }
#define AREA_LOG_ERROR(area, ...)                                              \
    if (krit::Log::defaultLevel < LogLevel::Error ||                           \
        krit::Log::level(std::hash<std::string_view>()(area)) <=               \
            LogLevel::Error) {                                                 \
        Log::error(area, __VA_ARGS__);                                         \
    }
#define AREA_LOG_OUTPUT(area, ...)                                             \
    if (krit::Log::defaultLevel < LogLevel::Output ||                          \
        krit::Log::level(std::hash<std::string_view>()(area)) <=               \
            LogLevel::Output) {                                                \
        Log::output(area, __VA_ARGS__);                                        \
    }
#define AREA_LOG_FATAL(area, ...)                                              \
    if (krit::Log::defaultLevel < LogLevel::Fatal ||                           \
        krit::Log::level(std::hash<std::string_view>()(area)) <=               \
            LogLevel::Fatal) {                                                 \
        Log::fatal(area, __VA_ARGS__);                                         \
    }

enum LogLevel {
    Debug,
    Info,
    Warn,
    Error,
    Output,
    Fatal,
    LogLevelCount,
};

using LogSink = std::function<void(LogLevel level, std::string_view area,
                                   const char *fmt, va_list args)>;

struct Log {
    static LogLevel defaultLevel;
    static std::unordered_map<size_t, LogLevel> levelMap;
    static std::vector<LogSink> logSinks;

    static LogLevel level(std::string_view area);
    static LogLevel level(size_t area);
    static void log(LogLevel level, std::string_view area, const char *fmt,
                    va_list args);
    static void consoleLog(LogLevel level, std::string_view area,
                           const char *fmt, va_list args);
    static void addLogSink(LogSink sink) { logSinks.push_back(sink); }

#define DEFINE_LOG_METHOD(name, level)                                         \
    static void name(std::string_view area, const char *fmt, ...) {            \
        va_list args;                                                          \
        va_start(args, fmt);                                                   \
        log(LogLevel::level, area, fmt, args);                                 \
        va_end(args);                                                          \
    }
    DEFINE_LOG_METHOD(debug, Debug)
    DEFINE_LOG_METHOD(info, Info)
    DEFINE_LOG_METHOD(warn, Warn)
    DEFINE_LOG_METHOD(error, Error)
    DEFINE_LOG_METHOD(output, Output)
    DEFINE_LOG_METHOD(fatal, Fatal)
#undef DEFINE_LOG_METHOD
};

}

#endif
