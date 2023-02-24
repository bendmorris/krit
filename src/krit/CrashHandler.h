#ifndef KRIT_CRASHHANDLER
#define KRIT_CRASHHANDLER

namespace krit {

struct CrashHandler {
    static void init();
    [[noreturn]] static void exit(int code);
};

}

#endif
