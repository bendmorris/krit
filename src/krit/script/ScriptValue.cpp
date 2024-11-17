#include "ScriptValue.h"
#include "krit/Engine.h"
#include "krit/utils/Log.h"

namespace krit {

JSValue getCachedInstance(JSContext *ctx, int classId, const void *p) {
    ScriptEngine *engine =
        static_cast<ScriptEngine *>(JS_GetContextOpaque(ctx));
    auto found = engine->instances.find(std::make_pair(classId, p));
    if (found == engine->instances.end()) {
        return JS_UNDEFINED;
    }
    return JS_DupValue(ctx, found->second);
}
void setCachedInstance(JSContext *ctx, int classId, const void *p, JSValue val) {
    ScriptEngine *engine =
        static_cast<ScriptEngine *>(JS_GetContextOpaque(ctx));
    engine->instances[std::make_pair(classId, p)] = val;
}

/**
 * If a function returns a JSValue by value, we assume it's a live, newly
 * allocated value. No one else should retain it without increasing the ref
 * count.
 */
JSValue ScriptValueToJs<JSValue>::valueToJs(JSContext *ctx, JSValue val) {
    // engine->script.dumpBacktrace(stderr);
    return val;
}

/**
 * If a function returns a reference to an existing JSValue, we need to increase
 * the reference count before returning as they still hold it.
 */
JSValue &ScriptValueToJs<JSValue &>::valueToJs(JSContext *ctx, JSValue &val) {
    JS_DupValue(ctx, val);
    return val;
}

// c string from JS value
const char *ScriptValueFromJs<const char *>::valueFromJs(JSContext *ctx,
                                                         JSValue val) {
    // size_t len;
    // const char *s = JS_ToCStringLen(ctx, &len, val);
    // char *result = new char[len + 1];
    // strncpy(result, s, len);
    // result[len] = 0;
    // JS_FreeCString(ctx, s);
    // return result;
    engine->script.dumpBacktrace(stderr);
    panic("char * is not supported by ScriptValue, because it will leak; use "
          "std::string instead");
}

}
