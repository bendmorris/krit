#ifndef KRIT_SCRIPT_SCRIPTVALUE
#define KRIT_SCRIPT_SCRIPTVALUE

#include "krit/script/ScriptClass.h"
#include "krit/script/ScriptType.h"
#include "quickjs.h"
#include <cassert>
#include <functional>
#include <memory>
#include <optional>
#include <stddef.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace krit {

struct ScriptEngine;

// template <typename T> struct Partial<T> {
//     static void init(JSContext *ctx, T *val, JSValue obj) {
//         val = T();

//     }
// }

template <typename T> struct is_class_type {
    static constexpr bool value = std::is_class<T>::value;
};

template <typename T> struct is_class_type<std::unique_ptr<T>> {
    static constexpr bool value = false;
};

template <typename T> struct is_class_type<std::shared_ptr<T>> {
    static constexpr bool value = false;
};

template <typename T> struct is_class_type<std::vector<T>> {
    static constexpr bool value = false;
};

template <typename T> struct is_class_type<const std::vector<T>> {
    static constexpr bool value = false;
};

template <> struct is_class_type<std::string> {
    static constexpr bool value = false;
};

template <> struct is_class_type<std::string_view> {
    static constexpr bool value = false;
};

template <typename T> struct is_basic_type {
    static constexpr bool value = !std::is_class<T>::value;
};

template <typename T, typename enable = void> struct ScriptValueFromJs {
    static T valueFromJs(JSContext *ctx, JSValue val);
};

// valueFromJs is defined for: basic types, class types, class type references,
// class type pointers

template <> struct ScriptValueFromJs<JSValue> {
    static JSValue valueFromJs(JSContext *ctx, JSValue val) {
        return JS_DupValue(ctx, val);
    }
};

// bool
template <> struct ScriptValueFromJs<bool> {
    static bool valueFromJs(JSContext *ctx, JSValue val) {
        return JS_ToBool(ctx, val);
    }
};

// signed integral type or enum from JS value
template <typename T>
struct ScriptValueFromJs<
    T, typename std::enable_if<
           std::is_enum<typename std::remove_reference<T>::type>::value ||
           (std::is_signed<typename std::remove_reference<T>::type>::value &&
            std::is_integral<typename std::remove_reference<T>::type>::value)>::
           type> {
    static T valueFromJs(JSContext *ctx, JSValue val) {
        int32_t n;
        JS_ToInt32(ctx, &n, val);
        return static_cast<T>(n);
    }
};

// unsigned integral type from JS value
template <typename T>
struct ScriptValueFromJs<
    T, typename std::enable_if<std::is_unsigned<
           typename std::remove_reference<T>::type>::value>::type> {
    static T valueFromJs(JSContext *ctx, JSValue val) {
        uint32_t n;
        JS_ToUint32(ctx, &n, val);
        return static_cast<T>(n);
    }
};

// float type from JS value
template <typename T>
struct ScriptValueFromJs<
    T, typename std::enable_if<std::is_floating_point<
           typename std::remove_reference<T>::type>::value>::type> {
    static T valueFromJs(JSContext *ctx, JSValue val) {
        double n;
        JS_ToFloat64(ctx, &n, val);
        return n;
    }
};

// c string from JS value
template <> struct ScriptValueFromJs<const char *> {
    static const char *valueFromJs(JSContext *ctx, JSValue val) {
        return (const char *)JS_ToCString(ctx, val);
    }
};

// std::string from JS value
template <> struct ScriptValueFromJs<std::string> {
    static std::string valueFromJs(JSContext *ctx, JSValue val) {
        return std::string(
            ScriptValueFromJs<const char *>::valueFromJs(ctx, val));
    }
};

template <> struct ScriptValueFromJs<std::string &> {
    static std::string valueFromJs(JSContext *ctx, JSValue val) {
        return std::string(
            ScriptValueFromJs<const char *>::valueFromJs(ctx, val));
    }
};

template <> struct ScriptValueFromJs<const std::string &> {
    static std::string valueFromJs(JSContext *ctx, JSValue val) {
        return std::string(
            ScriptValueFromJs<const char *>::valueFromJs(ctx, val));
    }
};

// script class type from JS value
template <typename T>
struct ScriptValueFromJs<
    T, typename std::enable_if<is_class_type<T>::value>::type> {
    using T0 = typename std::remove_const<T>::type;
    static T valueFromJs(JSContext *ctx, JSValue val) {
        return ScriptValueFromJs<T &>::valueFromJs(ctx, val);
    }
};

template <typename T>
struct ScriptValueFromJs<
    T &, typename std::enable_if<is_class_type<T>::value>::type> {
    using T0 = typename std::remove_const<T>::type;
    static T &valueFromJs(JSContext *ctx, JSValue val) {
        assert(ScriptClass<T0>::generated());
        void *p = JS_GetOpaque(val, 0);
        assert(p);
        return *(T *)p;
    }
};

template <typename T>
struct ScriptValueFromJs<
    T *, typename std::enable_if<is_class_type<T>::value>::type> {
    using T0 = typename std::remove_const<T>::type;
    static T *valueFromJs(JSContext *ctx, JSValue val) {
        if (JS_IsUndefined(val) || JS_IsNull(val) || !JS_GetOpaque(val, 0)) {
            return nullptr;
        }
        return &ScriptValueFromJs<T &>::valueFromJs(ctx, val);
    }
};

// vectors
template <typename T> struct ScriptValueFromJs<std::vector<T>> {
    static std::vector<T> valueFromJs(JSContext *ctx, JSValue arr) {
        int len;
        JSValue length = JS_GetPropertyStr(ctx, arr, "length");
        JS_ToInt32(ctx, &len, length);
        std::vector<T> vec(len);
        for (int i = 0; i < len; ++i) {
            JSValue item = JS_GetPropertyUint32(ctx, arr, i);
            vec[i] = ScriptValueFromJs<T>::valueFromJs(ctx, item);
        }
        JS_FreeValue(ctx, length);
        return vec;
    }
};

// vectors
template <typename T> struct ScriptValueFromJs<std::vector<T>&&> {
    static std::vector<T> &&valueFromJs(JSContext *ctx, JSValue arr) {
        int len;
        JSValue length = JS_GetPropertyStr(ctx, arr, "length");
        JS_ToInt32(ctx, &len, length);
        std::vector<T> vec(len);
        for (int i = 0; i < len; ++i) {
            JSValue item = JS_GetPropertyUint32(ctx, arr, i);
            vec[i] = ScriptValueFromJs<T>::valueFromJs(ctx, item);
        }
        JS_FreeValue(ctx, length);
        return std::move(vec);
    }
};

template <typename T0> struct ScriptValueFromPartial {
    using T = typename remove_all<T0>::type;

    static T valueFromPartial(JSContext *ctx, JSValue partial) {
        assert(ScriptClass<T>::generated());
        void *opaque = JS_GetOpaque(partial, 0);
        if (opaque) {
            return *static_cast<T*>(opaque);
        } else {
            T val;
            ScriptClass<T>::populateFromPartial(ctx, val, partial);
            return val;
        }
    }
};

// valueToJs is defined for: basic types, basic type
// references, basic type pointers, class type references, class type pointers
template <typename T, typename enable = void> struct ScriptValueToJs {
    static JSValue valueToJs(JSContext *ctx, T val);
};

template <> struct ScriptValueToJs<JSValue> {
    static JSValue valueToJs(JSContext *ctx, const JSValue &val) {
        JS_DupValue(ctx, val);
        return val;
    }
};

template <typename T>
struct ScriptValueToJs<T &,
                       typename std::enable_if<is_basic_type<T>::value>::type> {
    static JSValue valueToJs(JSContext *ctx, T &val) {
        return ScriptValueToJs<T>::valueToJs(ctx, val);
    }
};

template <typename T>
struct ScriptValueToJs<const T &,
                       typename std::enable_if<is_basic_type<T>::value>::type> {
    static JSValue valueToJs(JSContext *ctx, const T &val) {
        return ScriptValueToJs<T>::valueToJs(ctx, val);
    }
};

template <typename T>
struct ScriptValueToJs<T *,
                       typename std::enable_if<is_basic_type<T>::value>::type> {
    static JSValue valueToJs(JSContext *ctx, T *val) {
        return ScriptValueToJs<T>::valueToJs(ctx, *val);
    }
};

// bool
template <> struct ScriptValueToJs<bool> {
    static JSValue valueToJs(JSContext *ctx, bool val) {
        return JS_NewBool(ctx, val);
    }
};

// signed integral type or enum to JS value
template <typename T>
struct ScriptValueToJs<
    T, typename std::enable_if<std::is_enum<T>::value ||
                               (std::is_signed<T>::value &&
                                std::is_integral<T>::value)>::type> {
    static JSValue valueToJs(JSContext *ctx, T val) {
        return JS_NewInt32(ctx, val);
    }
};

// unsigned integral type to JS value
template <typename T>
struct ScriptValueToJs<
    T, typename std::enable_if<std::is_unsigned<T>::value>::type> {
    static JSValue valueToJs(JSContext *ctx, T val) {
        return JS_NewUint32(ctx, val);
    }
};

// float type to JS value
template <typename T>
struct ScriptValueToJs<
    T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
    static JSValue valueToJs(JSContext *ctx, T val) {
        return JS_NewFloat64(ctx, val);
    }
};

// c string to JS value
template <> struct ScriptValueToJs<const char *> {
    static JSValue valueToJs(JSContext *ctx, const char *val) {
        return JS_NewString(ctx, val);
    }
};

// std::string to JS value
template <> struct ScriptValueToJs<std::string> {
    static JSValue valueToJs(JSContext *ctx, const std::string &val) {
        return JS_NewStringLen(ctx, val.c_str(), val.size());
    }
};
template <> struct ScriptValueToJs<std::string &> {
    static JSValue valueToJs(JSContext *ctx, const std::string &val) {
        return JS_NewStringLen(ctx, val.c_str(), val.size());
    }
};
template <> struct ScriptValueToJs<const std::string &> {
    static JSValue valueToJs(JSContext *ctx, const std::string &val) {
        return JS_NewStringLen(ctx, val.c_str(), val.size());
    }
};

template <> struct ScriptValueToJs<std::string_view> {
    static JSValue valueToJs(JSContext *ctx, const std::string_view &val) {
        return JS_NewStringLen(ctx, val.data(), val.size());
    }
};

// script class type to JS value
template <typename T>
struct ScriptValueToJs<T,
                       typename std::enable_if<is_class_type<T>::value>::type> {
    static JSValue valueToJs(JSContext *ctx, const T &val) {
        assert(ScriptClass<T>::generated());
        JSValue obj = JS_NewObjectClass(ctx, ScriptClass<T>::classId());
        JS_SetOpaque(obj, (void *)(&val));
        return obj;
    }
};

template <typename T>
struct ScriptValueToJs<T &,
                       typename std::enable_if<is_class_type<T>::value>::type> {
    static JSValue valueToJs(JSContext *ctx, const T &val) {
        assert(ScriptClass<T>::generated());
        JSValue obj = JS_NewObjectClass(ctx, ScriptClass<T>::classId());
        JS_SetOpaque(obj, (void *)(&val));
        return obj;
    }
};

template <typename T>
struct ScriptValueToJs<T *,
                       typename std::enable_if<is_class_type<T>::value>::type> {
    static JSValue valueToJs(JSContext *ctx, const T *val) {
        assert(ScriptClass<T>::generated());
        if (!val) {
            return JS_NULL;
        }
        JSValue obj = JS_NewObjectClass(ctx, ScriptClass<T>::classId());
        JS_SetOpaque(obj, (void *)(val));
        return obj;
    }
};

// vector
template <typename T> struct ScriptValueToJs<std::vector<T>> {
    static JSValue valueToJs(JSContext *ctx, const std::vector<T> &v) {
        return ScriptValueToJs<const std::vector<T> &>::valueToJs(ctx, v);
    }
};
template <typename T> struct ScriptValueToJs<std::vector<T> &> {
    static JSValue valueToJs(JSContext *ctx, const std::vector<T> &v) {
        return ScriptValueToJs<const std::vector<T> &>::valueToJs(ctx, v);
    }
};
template <typename T> struct ScriptValueToJs<const std::vector<T> &> {
    static JSValue valueToJs(JSContext *ctx, const std::vector<T> &v) {
        JSValue arr = JS_NewArray(ctx);
        JSValue push = JS_GetPropertyStr(ctx, arr, "push");
        for (size_t i = 0; i < v.size(); ++i) {
            JSValue arg = ScriptValueToJs<T>::valueToJs(ctx, v[i]);
            JSValue ret = JS_Call(ctx, push, arr, 1, &arg);
            JS_FreeValue(ctx, ret);
            JS_FreeValue(ctx, arg);
        }
        JS_FreeValue(ctx, push);
        return arr;
    }
};

// template <>
// JSValue ScriptValueToJs<JSValue>::valueToJs(JSContext *ctx, const JSValue
// &val); template <> JSValue ScriptValueToJs<JSValue *>::valueToJs(JSContext
// *ctx, JSValue *const &val); template <> JSValue
// ScriptValueFromJs<JSValue>::valueFromJs(JSContext *ctx, JSValue val);

struct ScopedScriptValue {
    JSContext *ctx;
    JSValue val;

    ScopedScriptValue(JSContext *ctx, JSValue val) : ctx(ctx), val(val) {}
    ~ScopedScriptValue() { JS_FreeValue(ctx, val); }
};
}

// make sure we include the ScriptEngine-dependent specializations as well
#include "krit/script/ScriptEngine.h"

#endif
