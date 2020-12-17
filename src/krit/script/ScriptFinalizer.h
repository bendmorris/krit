#ifndef KRIT_SCRIPT_FINALIZER
#define KRIT_SCRIPT_FINALIZER

namespace krit {

struct ScriptEngine;

struct ScriptFinalizer {
    static void init(ScriptEngine *engine);
};

}

#endif
