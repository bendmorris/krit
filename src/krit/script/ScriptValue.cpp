#include "krit/script/ScriptValue.h"
#include <string>

namespace krit {

template<> JSValue ScriptValue<bool>::valueToJs(JSContext *ctx, const bool &arg) { return JS_NewBool(ctx, arg); }
template<> bool ScriptValue<bool>::jsToValue(JSContext *ctx, JSValue arg) { return JS_ToBool(ctx, arg); }

template<> JSValue ScriptValue<int>::valueToJs(JSContext *ctx, const int &arg) { return JS_NewInt32(ctx, arg); }
template<> int ScriptValue<int>::jsToValue(JSContext *ctx, JSValue arg) { int dest; JS_ToInt32(ctx, &dest, arg); return dest; }

template<> JSValue ScriptValue<float>::valueToJs(JSContext *ctx, const float &arg) { return JS_NewFloat64(ctx, arg); }
template<> float ScriptValue<float>::jsToValue(JSContext *ctx, JSValue arg) { double d; JS_ToFloat64(ctx, &d, arg); return d; }

template<> JSValue ScriptValue<double>::valueToJs(JSContext *ctx, const double &arg) { return JS_NewFloat64(ctx, arg); }
template<> double ScriptValue<double>::jsToValue(JSContext *ctx, JSValue arg) { double d; JS_ToFloat64(ctx, &d, arg); return d; }

template<> JSValue ScriptValue<JSValue>::valueToJs(JSContext *ctx, const JSValue &arg) { JS_DupValue(ctx, arg); return arg; }
template<> JSValue ScriptValue<JSValue>::jsToValue(JSContext *ctx, JSValue arg) { return arg; }

template <> JSValue ScriptValue<char*>::valueToJs(JSContext *ctx, char * const &s) { return JS_NewString(ctx, s); }
template <> char *ScriptValue<char*>::jsToValue(JSContext *ctx, JSValue arg) { return (char*)JS_ToCString(ctx, arg); }

template <> JSValue ScriptValue<std::string>::valueToJs(JSContext *ctx, const std::string &s) { return JS_NewString(ctx, s.c_str()); }
template <> std::string ScriptValue<std::string>::jsToValue(JSContext *ctx, JSValue arg) {
    return std::string(ScriptValue<char*>::jsToValue(ctx, arg));
}

// template <typename T> JSValue valueToJs(JSContext *ctx, const std::unordered_map<std::string, T> &s) {
//     JSValue obj = JS_NewObject(ctx);
//     for (auto it = s.begin(); it != s.end(); ++it) {
//         JS_SetPropertyStr(ctx, obj, it->first.c_str(), valueToJs(ctx, &it->second));
//     }
//     return obj;
// }

// template <typename T> JSValue valueToJs(JSContext *ctx, const std::vector<T> &v) {
//     JSValue arr = JS_NewArray(ctx);
//     JSValue push = JS_GetPropertyStr(ctx, arr, "push");
//     for (size_t i = 0; i < v->size(); ++i) {
//         JSValue arg = valueToJs(ctx, &v[i]);
//         JS_Call(ctx, push, arr, 1, &arg);
//     }
//     JS_FreeValue(ctx, push);
//     return arr;
// }

}