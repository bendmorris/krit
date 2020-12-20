#include "krit/script/ScriptValue.h"

namespace krit {

template<> JSValue ScriptValue<JSValue>::valueToJs(JSContext *ctx, const JSValue &val ) { JS_DupValue(ctx, val ); return val ; }
template<> JSValue ScriptValue<JSValue*>::valueToJs(JSContext *ctx, JSValue * const &val) { if (!val) return JS_UNDEFINED; JS_DupValue(ctx, *val ); return *val ; }
template<> JSValue ScriptValue<JSValue>::jsToValue(JSContext *ctx, JSValue val ) { return val ; }

template<> JSValue ScriptValue<bool>::valueToJs(JSContext *ctx, const bool &val ) { return JS_NewBool(ctx, val ); }
template<> JSValue ScriptValue<bool*>::valueToJs(JSContext *ctx, bool * const &val) { return !val ? JS_UNDEFINED : JS_NewBool(ctx, *val ); }
template<> bool ScriptValue<bool>::jsToValue(JSContext *ctx, JSValue val ) { return JS_ToBool(ctx, val ); }

template<> JSValue ScriptValue<int>::valueToJs(JSContext *ctx, const int &val ) { return JS_NewInt32(ctx, val ); }
template<> JSValue ScriptValue<int*>::valueToJs(JSContext *ctx, int * const &val) { return !val ? JS_UNDEFINED : JS_NewInt32(ctx, *val ); }
template<> int ScriptValue<int>::jsToValue(JSContext *ctx, JSValue val ) { int dest; JS_ToInt32(ctx, &dest, val ); return dest; }

template<> JSValue ScriptValue<float>::valueToJs(JSContext *ctx, const float &val ) { return JS_NewFloat64(ctx, val ); }
template<> JSValue ScriptValue<float*>::valueToJs(JSContext *ctx, float * const &val) { return !val ? JS_UNDEFINED : JS_NewFloat64(ctx, *val ); }
template<> float ScriptValue<float>::jsToValue(JSContext *ctx, JSValue val ) { double d; JS_ToFloat64(ctx, &d, val ); return d; }

template<> JSValue ScriptValue<double>::valueToJs(JSContext *ctx, const double &val ) { return JS_NewFloat64(ctx, val ); }
template<> JSValue ScriptValue<double*>::valueToJs(JSContext *ctx, double * const &val) { return !val ? JS_UNDEFINED : JS_NewFloat64(ctx, *val ); }
template<> double ScriptValue<double>::jsToValue(JSContext *ctx, JSValue val ) { double d; JS_ToFloat64(ctx, &d, val ); return d; }

template <> JSValue ScriptValue<char*>::valueToJs(JSContext *ctx, char * const &s) { return JS_NewString(ctx, s); }
template <> char *ScriptValue<char*>::jsToValue(JSContext *ctx, JSValue val ) { return (char*)JS_ToCString(ctx, val ); }

template <> JSValue ScriptValue<std::string>::valueToJs(JSContext *ctx, const std::string &s) { return JS_NewString(ctx, s.c_str()); }
template <> JSValue ScriptValue<std::string*>::valueToJs(JSContext *ctx, std::string * const &s) { return !s ? JS_UNDEFINED : JS_NewString(ctx, s->c_str()); }
template <> std::string ScriptValue<std::string>::jsToValue(JSContext *ctx, JSValue val ) {
    return std::string(ScriptValue<char*>::jsToValue(ctx, val ));
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