#ifndef KRIT_UTILS_LOG
#define KRIT_UTILS_LOG

#include <cstdarg>
#include <cstdio>
#include <unistd.h>

namespace krit {

enum LogLevel {
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    Success,
    LogLevelCount,
};

struct Log {
    static const char *abbreviations[LogLevelCount];
    static const char *logOpen[LogLevelCount];
    static const char *logClose;
    static LogLevel level;

    static void log(LogLevel level, const char *fmt, va_list args);

    #define DEFINE_LOG_METHOD(name, level) static void name(const char *fmt, ...) { va_list args; va_start(args, fmt); log(LogLevel::level, fmt, args); va_end(args); }
    DEFINE_LOG_METHOD(debug, Debug)
    DEFINE_LOG_METHOD(info, Info)
    DEFINE_LOG_METHOD(warn, Warn)
    DEFINE_LOG_METHOD(error, Error)
    DEFINE_LOG_METHOD(fatal, Fatal)
    DEFINE_LOG_METHOD(success, Success)
    #undef DEFINE_LOG_METHOD
};

}

#endif
