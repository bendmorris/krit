#include "krit/utils/Log.h"

#include <cstdio>
#include <unistd.h>

namespace krit {

const char *Log::abbreviations[LogLevelCount] = {"DBG", "INF", "WRN", "ERR",
                                                 "!!!"};
const char *Log::logOpen[LogLevelCount] = {"", "\u001b[34m", "\u001b[33m",
                                           "\u001b[31m", "\u001b[35;1m"};
const char *Log::logClose = "\u001b[0m";

LogLevel Log::level = LogLevel::Error;
static bool console = isatty(fileno(stdout));

void Log::log(LogLevel level, const char *fmt, va_list args) {
    if (Log::level > level) {
        return;
    }
    if (console) {
        printf("%s", logOpen[level]);
    }

    printf("[%s] ", abbreviations[level]);
    vprintf(fmt, args);
    if (console) {
        printf("%s", logClose);
    }
    puts("");
}

}
