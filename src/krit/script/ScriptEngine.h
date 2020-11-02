#ifndef KRIT_SCRIPT_SCRIPTENGINE
#define KRIT_SCRIPT_SCRIPTENGINE

#include "krit/ecs/Entity.h"
#include "quickjs.h"
#include <cstring>
#include <string>
#include <vector>

#define JS_FUNC(n) static JSValue js_##n(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
#define JS_GLOBAL_FUNC(n, a) JS_SetPropertyStr(ctx, globalObj, #n, JS_NewCFunction(ctx, js_##n, #n, a))
#define JS_GLOBAL_FUNC2(n, f, a) JS_SetPropertyStr(ctx, globalObj, #n, JS_NewCFunction(ctx, f, #n, a))
#define JS_METHOD(obj, n, a) JS_SetPropertyStr(ctx, obj, #n, JS_NewCFunction(ctx, js_##n, #n, a))

namespace krit {

template <typename T> JSValue _valueToJS(JSContext *ctx, T val);
template <typename T> void _valueFromJS(JSContext *ctx, T *dest, JSValue val);

template <> inline void _valueFromJS(JSContext *ctx, void *dest, JSValue arg) { }
template <> inline JSValue _valueToJS(JSContext *ctx, bool arg) { return JS_NewBool(ctx, arg); }
template <> inline void _valueFromJS(JSContext *ctx, bool *dest, JSValue arg) { *dest = JS_ToBool(ctx, arg); }
template <> inline JSValue _valueToJS(JSContext *ctx, int arg) { return JS_NewInt32(ctx, arg); }
template <> inline void _valueFromJS(JSContext *ctx, int *dest, JSValue arg) { JS_ToInt32(ctx, dest, arg); }
template <> inline JSValue _valueToJS(JSContext *ctx, double arg) { return JS_NewFloat64(ctx, arg); }
template <> inline void _valueFromJS(JSContext *ctx, double *dest, JSValue arg) { JS_ToFloat64(ctx, dest, arg); }
template <> inline JSValue _valueToJS(JSContext *ctx, JSValue arg) { return arg; }
template <> inline void _valueFromJS(JSContext *ctx, JSValue *dest, JSValue arg) { JS_DupValue(ctx, arg); *dest = arg; }
template <> inline JSValue _valueToJS(JSContext *ctx, char *s) { return JS_NewString(ctx, s); }
template <> inline void _valueFromJS(JSContext *ctx, char **s, JSValue arg) {
    const char *val = JS_ToCString(ctx, arg);
    *s = new char[strlen(val) + 1];
    strcpy(*s, val);
}

template <typename Head, typename... Tail> void _unpackCallArgs(JSContext *ctx, JSValue *args, Head head, Tail... tail) {
    _unpackCallArgs<Head>(ctx, args, head);
    _unpackCallArgs<Tail...>(ctx, &args[1], tail...);
}
template <typename Head> void _unpackCallArgs(JSContext *ctx, JSValue *args, Head head) {
    args[0] = _valueToJS<Head>(ctx, head);
}
template <typename Head, typename... Tail> void _freeArgs(JSContext *ctx, JSValue *args, Head head, Tail... tail) {
    _freeArgs<Head>(ctx, args);
    _freeArgs<Tail...>(ctx, args[1]);
}
template <typename Head> void _freeArgs(JSContext *ctx, JSValue *args, Head head) {
    JS_FreeValue(ctx, args[0]);
}

/**
 * ScriptEngine currently assumes the use of QuickJS.
 *
 * Before constructing a ScriptEngine,
 *
 * - Create a runtime with `JS_NewRuntime()` and assign to `ScriptEngine::rt`.
 *   Since QuickJS uses stack position, this should be done as low in the
 *   stack as possible, e.g. from `main()`.
 * - Initialize any script classes.
 *
 * There should be one ScriptEngine instance per JS context. After constructing
 * an instance, initialize any special bridge functions or objects.
 *
 * Evaluate a JS file with `eval` and call functions using the various `call`
 * methods.
 */
struct ScriptEngine {
    static JSRuntime *rt;

    JSContext *ctx = nullptr;
    JSValue exports;
    void *userData;

    ScriptEngine();
    ~ScriptEngine();

    void eval(const std::string &scriptName, const std::string &src) {
        this->eval(scriptName.c_str(), src.c_str(), src.length());
    }
    void eval(const char *scriptName, const char *src, size_t len);
    char *evalToString(const std::string &scriptName, const std::string &src) {
        return this->evalToString(scriptName, src.c_str(), src.length());
    }
    char *evalToString(const std::string &scriptName, const char *src, size_t len);

    /**
     * Passing `functionName` as a string.
     */
    template <typename ReturnValue, typename... ArgTypes> void call(const std::string &functionName, ReturnValue *dest, ArgTypes... args) {
        return this->call<ReturnValue, ArgTypes...>(functionName.c_str(), dest, args...);
    }

    /**
     * Passing `functionName` as a const char *.
     */
    template <typename ReturnValue, typename... ArgTypes> void call(const char *functionName, ReturnValue *dest, ArgTypes... args) {
        JSValue func = JS_GetPropertyStr(ctx, exports, functionName);
        this->call<ReturnValue, ArgTypes...>(func, dest, args...);
        JS_FreeValue(ctx, func);
    }

    template <typename ReturnValue, typename... ArgTypes> void call(JSValue func, ReturnValue *destination, ArgTypes... args) {
        JSValue jsArgs[sizeof...(ArgTypes)];
        _unpackCallArgs<ArgTypes...>(this->ctx, jsArgs, args...);

        JSValue jsResult = JS_Call(ctx, func, JS_UNDEFINED, sizeof...(ArgTypes), jsArgs);
        checkForErrors();
        if (destination) {
            _valueFromJS<ReturnValue>(ctx, destination, jsResult);
        }
        JS_FreeValue(ctx, jsResult);
        _freeArgs<ArgTypes...>(ctx, jsArgs, args...);
        update();
    }

    void update();
    void checkForErrors();

    template <typename T> T *data() { return static_cast<T*>(this->userData); }
};

}

#endif
