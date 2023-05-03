#ifndef KRIT_SCRIPT_SCRIPTCLASS
#define KRIT_SCRIPT_SCRIPTCLASS

#include <quickjs.h>

namespace krit {

struct ScriptEngine;

template <typename T> struct ScriptClass {
    static JSClassID classId() {
        static JSClassID id = 0;
        if (!id) {
            JS_NewClassID(&id);
        }
        return id;
    }

    static void populateFromPartial(JSContext *ctx, T &val, JSValue partial);
    static void init(ScriptEngine *engine);

    // this is used in static assertions and will cause a link failure in case of a template type
    // incorrectly matching to a specialization for script classes
    static bool generated();
};

}

#endif
