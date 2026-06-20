#include "krit/utils/Panic.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace krit {

void panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vpanic(fmt, args);
    va_end(args);
}

void vpanic(const char *fmt, va_list args) {
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    fflush(stderr);
    std::abort();
}

}
