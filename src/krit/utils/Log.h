#ifndef KRIT_UTILS_LOG
#define KRIT_UTILS_LOG

#include <cstdarg>
#include <cstdio>
#include <unistd.h>

namespace krit {

#define LOG_DEBUG(...)                                                         \
    if (krit::Log::level <= LogLevel::Debug) {                                 \
        Log::debug(__VA_ARGS__);                                               \
    }
#define LOG_INFO(...)                                                          \
    if (krit::Log::level <= LogLevel::Info) {                                  \
        Log::info(__VA_ARGS__);                                                \
    }
#define LOG_WARN(...)                                                          \
    if (krit::Log::level <= LogLevel::Warn) {                                  \
        Log::warn(__VA_ARGS__);                                                \
    }
#define LOG_ERROR(...)                                                         \
    if (krit::Log::level <= LogLevel::Error) {                                 \
        Log::error(__VA_ARGS__);                                               \
    }
#define LOG_FATAL(...)                                                         \
    if (krit::Log::level <= LogLevel::Fatal) {                                 \
        Log::fatal(__VA_ARGS__);                                               \
    }

enum LogLevel {
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    LogLevelCount,
};

struct Log {
    static const char *abbreviations[LogLevelCount];
    static const char *logOpen[LogLevelCount];
    static const char *logClose;
    static LogLevel level;

    static void log(LogLevel level, const char *fmt, va_list args);

#define DEFINE_LOG_METHOD(name, level)                                         \
    static void name(const char *fmt, ...) {                                   \
        va_list args;                                                          \
        va_start(args, fmt);                                                   \
        log(LogLevel::level, fmt, args);                                       \
        va_end(args);                                                          \
    }
    DEFINE_LOG_METHOD(debug, Debug)
    DEFINE_LOG_METHOD(info, Info)
    DEFINE_LOG_METHOD(warn, Warn)
    DEFINE_LOG_METHOD(error, Error)
    DEFINE_LOG_METHOD(fatal, Fatal)
#undef DEFINE_LOG_METHOD
};

}

#endif
