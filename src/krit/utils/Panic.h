#include <stdarg.h>

namespace krit {

[[noreturn]] void panic(const char *fmt...);
[[noreturn]] void vpanic(const char *fmt, va_list args);

}
