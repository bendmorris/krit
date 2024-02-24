#include "krit/utils/Log.h"

#include <chrono>
#include <cstdio>
#include <math.h>
#include <unistd.h>

namespace krit {

const char *Log::abbreviations[LogLevelCount] = {"DBG", "INF", "WRN", "ERR",
                                                 "!!!"};
const char *Log::logOpen[LogLevelCount] = {"", "\u001b[34m", "\u001b[33m",
                                           "\u001b[31m", "\u001b[35;1m"};
const char *Log::logClose = "\u001b[0m";

const auto start_time = std::chrono::high_resolution_clock::now();

LogLevel Log::level = LogLevel::Error;
static bool console = isatty(fileno(stdout));

void Log::log(LogLevel level, const char *fmt, va_list args) {
    if (Log::level > level) {
        return;
    }
    double s = std::chrono::duration<double, std::milli>(
                   std::chrono::high_resolution_clock::now() - start_time)
                   .count() /
               1000;
    int m = s / 60;
    s = fmod(s, 60);
    int h = m / 60;
    m = fmod(m, 60);
    printf("%02i:%02i:%06.3f ", h, m, s);
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
