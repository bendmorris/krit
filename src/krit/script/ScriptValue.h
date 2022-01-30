#ifndef KRIT_SCRIPT_SCRIPTVALUE
#define KRIT_SCRIPT_SCRIPTVALUE

#include "quickjs.h"
#include <functional>
#include <stddef.h>
#include <string>
#include <vector>

namespace krit {

template <typename T> struct ScriptValue {
    static JSValue valueToJs(JSContext *ctx, const T &val);
    static T jsToValue(JSContext *ctx, JSValue val);
};

template <>
JSValue ScriptValue<JSValue>::valueToJs(JSContext *ctx, const JSValue &val);
template <>
JSValue ScriptValue<JSValue *>::valueToJs(JSContext *ctx, JSValue *const &val);
template <>
JSValue ScriptValue<JSValue>::jsToValue(JSContext *ctx, JSValue val);
template <>
JSValue ScriptValue<bool>::valueToJs(JSContext *ctx, const bool &val);
template <>
JSValue ScriptValue<bool *>::valueToJs(JSContext *ctx, bool *const &val);
template <> bool ScriptValue<bool>::jsToValue(JSContext *ctx, JSValue val);
template <> JSValue ScriptValue<int>::valueToJs(JSContext *ctx, const int &val);
template <>
JSValue ScriptValue<int *>::valueToJs(JSContext *ctx, int *const &val);
template <> int ScriptValue<int>::jsToValue(JSContext *ctx, JSValue val);
template <>
JSValue ScriptValue<float>::valueToJs(JSContext *ctx, const float &val);
template <>
JSValue ScriptValue<float *>::valueToJs(JSContext *ctx, float *const &val);
template <> float ScriptValue<float>::jsToValue(JSContext *ctx, JSValue val);
template <>
JSValue ScriptValue<double>::valueToJs(JSContext *ctx, const double &val);
template <>
JSValue ScriptValue<double *>::valueToJs(JSContext *ctx, double *const &val);
template <> double ScriptValue<double>::jsToValue(JSContext *ctx, JSValue val);
template <>
JSValue ScriptValue<const char *>::valueToJs(JSContext *ctx,
                                             const char *const &val);
template <>
const char *ScriptValue<const char *>::jsToValue(JSContext *ctx, JSValue val);
template <>
JSValue ScriptValue<std::string>::valueToJs(JSContext *ctx,
                                            const std::string &val);
template <>
JSValue ScriptValue<std::string *>::valueToJs(JSContext *ctx,
                                              std::string *const &val);
template <>
std::string ScriptValue<std::string>::jsToValue(JSContext *ctx, JSValue val);

// namespace {

// template <typename Head>
// void _unpackCallArgs(JSContext *ctx, JSValue *args, Head &head) {
//     args[0] = ScriptValue<Head>::valueToJs(ctx, head);
// }
// template <typename Head, typename... Tail>
// void _unpackCallArgs(JSContext *ctx, JSValue *args, Head &head,
//                      Tail &... tail) {
//     _unpackCallArgs<Head>(ctx, args, head);
//     _unpackCallArgs<Tail...>(ctx, &args[1], tail...);
// }

// }

// template <typename... Args>
// std::function<void(Args...)> jsToValue(JSContext *ctx, JSValue val) {
//     return [=](Args... args) {
//         JSValue jsArgs[sizeof...(Args)];
//         _unpackCallArgs(ctx, jsArgs, args...);
//         JS_Call(ctx, val, JS_UNDEFINED, sizeof...(Args), &jsArgs);
//     };
// }

template <typename T> struct ScriptValue<std::vector<T>> {
    static JSValue valueToJs(JSContext *ctx, const std::vector<T> &v) {
        JSValue arr = JS_NewArray(ctx);
        JSValue push = JS_GetPropertyStr(ctx, arr, "push");
        for (size_t i = 0; i < v.size(); ++i) {
            JSValue arg = ScriptValue<T>::valueToJs(ctx, v[i]);
            JSValue ret = JS_Call(ctx, push, arr, 1, &arg);
            JS_FreeValue(ctx, ret);
            JS_FreeValue(ctx, arg);
        }
        JS_FreeValue(ctx, push);
        return arr;
    }

    static std::vector<T> jsToValue(JSContext *ctx, JSValue arr) {
        int len;
        JSValue length = JS_GetPropertyStr(ctx, arr, "length");
        JS_ToInt32(ctx, &len, length);
        std::vector<T> vec(len);
        for (int i = 0; i < len; ++i) {
            JSValue item = JS_GetPropertyUint32(ctx, arr, i);
            vec[i] = ScriptValue<T>::jsToValue(ctx, item);
        }
        JS_FreeValue(ctx, length);
        return vec;
    }
};

struct ScopedScriptValue {
    JSContext *ctx;
    JSValue val;

    ScopedScriptValue(JSContext *ctx, JSValue val) : ctx(ctx), val(val) {}
    ~ScopedScriptValue() { JS_FreeValue(ctx, val); }
};

}

#endif
