#ifndef KRIT_SCRIPT_SCRIPTENGINE
#define KRIT_SCRIPT_SCRIPTENGINE

#include "krit/script/ScriptClass.h"
#include "krit/script/ScriptFinalizer.h"
#include "krit/script/ScriptValue.h"
#include "quickjs.h"
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace krit {

template <class T, std::size_t = sizeof(T)> std::true_type is_complete_impl(T *);
std::false_type is_complete_impl(...);

template <class T> using is_complete = decltype(is_complete_impl(std::declval<T*>()));

#define JS_FUNC(n) JSValue js_##n(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
#define JS_GLOBAL_FUNC(n, a) JS_SetPropertyStr(ctx, globalObj, #n, JS_NewCFunction(ctx, js_##n, #n, a))
#define JS_GLOBAL_FUNC2(n, f, a) JS_SetPropertyStr(ctx, globalObj, #n, JS_NewCFunction(ctx, f, #n, a))
#define JS_METHOD(obj, n, a) JS_SetPropertyStr(ctx, obj, #n, JS_NewCFunction(ctx, js_##n, #n, a))
#define GET_ENGINE ScriptEngine *engine = static_cast<ScriptEngine*>(JS_GetContextOpaque(ctx));

template <typename Head> void _unpackCallArgs(JSContext *ctx, JSValue *args, Head &head) {
    args[0] = ScriptValue<Head>::valueToJs(ctx, head);
}
template <typename Head, typename... Tail> void _unpackCallArgs(JSContext *ctx, JSValue *args, Head &head, Tail&... tail) {
    _unpackCallArgs<Head>(ctx, args, head);
    _unpackCallArgs<Tail...>(ctx, &args[1], tail...);
}

template <typename Head> void _freeArgs(JSContext *ctx, JSValue *args, Head &head) {
    JS_FreeValue(ctx, args[0]);
}
template <typename Head, typename... Tail> void _freeArgs(JSContext *ctx, JSValue *args, Head &head, Tail&... tail) {
    _freeArgs<Head>(ctx, args, head);
    _freeArgs<Tail...>(ctx, &args[1], tail...);
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

    static std::vector<JSClassID> classIds;

    void eval(const std::string &scriptName, const std::string &src) {
        this->eval(scriptName.c_str(), src.c_str(), src.length());
    }
    void eval(const char *scriptName, const char *src, size_t len);
    char *evalToString(const std::string &scriptName, const std::string &src) {
        return this->evalToString(scriptName, src.c_str(), src.length());
    }
    char *evalToString(const std::string &scriptName, const char *src, size_t len);

    template <typename ReturnValue> void callPut(ReturnValue &dest, JSValue func) {
        JSValue jsResult = JS_Call(ctx, func, JS_UNDEFINED, 0, nullptr);
        checkForErrors(jsResult);
        ScriptValue<ReturnValue>::jsToValue(ctx, dest, jsResult);
        JS_FreeValue(ctx, jsResult);
        update();
    }
    template <typename ReturnValue, typename Arg, typename... ArgTypes> void callPut(ReturnValue &dest, JSValue func, Arg &arg, ArgTypes&... args) {
        JSValue jsArgs[1 + sizeof...(ArgTypes)];
        _unpackCallArgs<Arg, ArgTypes...>(this->ctx, jsArgs, arg, args...);

        JSValue jsResult = JS_Call(ctx, func, JS_UNDEFINED, 1 + sizeof...(ArgTypes), jsArgs);
        checkForErrors(jsResult);
        ScriptValue<ReturnValue>::jsToValue(ctx, dest, jsResult);
        JS_FreeValue(ctx, jsResult);
        _freeArgs<Arg, ArgTypes...>(ctx, jsArgs, arg, args...);
        update();
    }
    template <typename ReturnValue, typename... ArgTypes> void callPut(const char *functionName, ReturnValue &dest, ArgTypes&... args) {
        JSValue func = JS_GetPropertyStr(ctx, exports, functionName);
        this->callPut<ReturnValue, ArgTypes...>(func, dest, args...);
        JS_FreeValue(ctx, func);
    }
    template <typename ReturnValue, typename... ArgTypes> void callPut(const std::string &functionName, ReturnValue &dest, ArgTypes&... args) {
        return this->callPut<ReturnValue, ArgTypes...>(functionName.c_str(), dest, args...);
    }


    template <typename ReturnValue> ReturnValue callReturn(JSValue func) {
        JSValue jsResult = JS_Call(ctx, func, JS_UNDEFINED, 0, nullptr);
        checkForErrors(jsResult);
        ReturnValue dest;
        ScriptValue<ReturnValue>::jsToValue(ctx, dest, jsResult);
        JS_FreeValue(ctx, jsResult);
        update();
        return dest;
    }
    template <typename ReturnValue, typename Arg, typename... ArgTypes> ReturnValue callReturn(JSValue func, Arg &arg, ArgTypes&... args) {
        JSValue jsArgs[1 + sizeof...(ArgTypes)];
        _unpackCallArgs<Arg, ArgTypes...>(this->ctx, jsArgs, arg, args...);
        JSValue jsResult = JS_Call(ctx, func, JS_UNDEFINED, 1 + sizeof...(ArgTypes), jsArgs);
        checkForErrors(jsResult);
        ReturnValue dest;
        ScriptValue<ReturnValue>::jsToValue(ctx, dest, jsResult);
        JS_FreeValue(ctx, jsResult);
        _freeArgs<Arg, ArgTypes...>(ctx, jsArgs, arg, args...);
        update();
        return dest;
    }
    template <typename ReturnValue, typename... ArgTypes> ReturnValue callReturn(const char *functionName, ArgTypes&... args) {
        JSValue func = JS_GetPropertyStr(ctx, exports, functionName);
        ReturnValue result = this->callReturn<ReturnValue, ArgTypes...>(func, args...);
        JS_FreeValue(ctx, func);
        return result;
    }
    template <typename ReturnValue, typename... ArgTypes> ReturnValue callReturn(const std::string &functionName, ArgTypes&... args) {
        return this->callReturn<ReturnValue, ArgTypes...>(functionName.c_str(), args...);
    }

    void callVoid(JSValue func) {
        JSValue jsResult = JS_Call(ctx, func, JS_UNDEFINED, 0, nullptr);
        checkForErrors(jsResult);
        JS_FreeValue(ctx, jsResult);
        update();
    }
    template <typename Arg, typename... ArgTypes> void callVoid(JSValue func, Arg& arg, ArgTypes&... args) {
        JSValue jsArgs[1 + sizeof...(ArgTypes)];
        _unpackCallArgs<Arg, ArgTypes...>(this->ctx, jsArgs, arg, args...);
        JSValue jsResult = JS_Call(ctx, func, JS_UNDEFINED, 1 + sizeof...(ArgTypes), jsArgs);
        checkForErrors(jsResult);
        JS_FreeValue(ctx, jsResult);
        _freeArgs<Arg, ArgTypes...>(ctx, jsArgs, arg, args...);
        update();
    }
    template <typename... ArgTypes> void callVoid(const char *functionName, ArgTypes&... args) {
        JSValue func = JS_GetPropertyStr(ctx, exports, functionName);
        this->callVoid<ArgTypes...>(func, args...);
        JS_FreeValue(ctx, func);
    }
    template <typename... ArgTypes> void callVoid(const std::string &functionName, ArgTypes&... args) {
        this->callVoid<ArgTypes...>(functionName.c_str(), args...);
    }

    void update();
    void checkForErrors();
    void checkForErrors(JSValue);

    void addFinalizer(JSValue obj, ScriptClass e);

    template <typename T> T *data() { return static_cast<T*>(this->userData); }

    friend struct ScriptFinalizer;

    private:
        JSValue finalizerSymbol;
};

template <int C> std::pair<const JSCFunctionListEntry *, size_t> scriptClassProtoFuncs();
template <int C> JSClassDef *scriptClassDef();

}

#endif
