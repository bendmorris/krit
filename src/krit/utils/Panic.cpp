#include "krit/utils/Panic.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace krit {

void panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vpanic(fmt, args);
}

void vpanic(const char *fmt, va_list args) {
    vfprintf(stderr, fmt, args);
    fputs("\n", stderr);
    ssize_t len = vsnprintf(NULL, 0, fmt, args);
    char *buf = static_cast<char *>(std::malloc(len + 1));
    vsnprintf(buf, len + 1, fmt, args);
    throw buf;
}

}
