#include "krit/utils/Panic.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace krit {

void panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fputs("\n", stderr);
    abort();
}

}
