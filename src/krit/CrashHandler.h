#pragma once

namespace krit {

struct CrashHandler {
    static void init();
    [[noreturn]] static void exit(int code);
};

}
