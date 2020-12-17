#ifndef KRIT_SCRIPT_SCRIPTVALUE
#define KRIT_SCRIPT_SCRIPTVALUE

#include "quickjs.h"
#include <string>

namespace krit {

template <typename T> struct ScriptValue {
    static JSValue valueToJs(JSContext *ctx, const T &val);
    static T jsToValue(JSContext *ctx, JSValue val);
};

template <> JSValue ScriptValue<JSValue>::valueToJs(JSContext *ctx, const JSValue &val);
template <> JSValue ScriptValue<JSValue>::jsToValue(JSContext *ctx, JSValue val);
template <> JSValue ScriptValue<bool>::valueToJs(JSContext *ctx, const bool &val);
template <> bool ScriptValue<bool>::jsToValue(JSContext *ctx, JSValue val);
template <> JSValue ScriptValue<int>::valueToJs(JSContext *ctx, const int &val);
template <> int ScriptValue<int>::jsToValue(JSContext *ctx, JSValue val);
template <> JSValue ScriptValue<float>::valueToJs(JSContext *ctx, const float &val);
template <> float ScriptValue<float>::jsToValue(JSContext *ctx, JSValue val);
template <> JSValue ScriptValue<double>::valueToJs(JSContext *ctx, const double &val);
template <> double ScriptValue<double>::jsToValue(JSContext *ctx, JSValue val);
template <> JSValue ScriptValue<char *>::valueToJs(JSContext *ctx, char * const &val);
template <> char *ScriptValue<char *>::jsToValue(JSContext *ctx, JSValue val);
template <> JSValue ScriptValue<std::string>::valueToJs(JSContext *ctx, const std::string &val);
template <> std::string ScriptValue<std::string>::jsToValue(JSContext *ctx, JSValue val);

}

#endif
