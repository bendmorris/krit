#pragma once

#include "krit/script/ObjectHeader.h"
#include "krit/script/ScriptClass.h"
#include "krit/script/ScriptType.h"
#include "quickjs.h"
#include <cassert>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <stddef.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace krit {

struct Promise;
struct ScriptEngine;

template <typename T, typename enable_if = void> struct TypeConverter {
    static JSValue valueToJs(JSContext *ctx, const T &val);
    static T valueFromJs(JSContext *ctx, const JSValue &val);
};

template <typename T> struct has_reference_specialization : std::false_type {};
template <typename T>
struct has_reference_specialization<std::unique_ptr<T>> : std::true_type {};
template <typename T>
struct has_reference_specialization<std::shared_ptr<T>> : std::true_type {};
template <typename T>
struct has_reference_specialization<std::vector<T>> : std::true_type {};
template <typename T>
struct has_reference_specialization<std::optional<T>> : std::true_type {};
template <typename T>
struct has_reference_specialization<std::function<T>> : std::true_type {};
template <>
struct has_reference_specialization<std::string> : std::true_type {};

template <typename T, typename enable_if = void>
struct convertible_from_js : std::false_type {};
template <typename T>
struct convertible_from_js<T,
                           std::void_t<decltype(TypeConverter<T>::valueFromJs)>>
    : std::true_type {};

/**
 * If a function returns a new JSValue, it is assumed to have had its ref count
 * incremented already and be "live".
 */

template <> struct TypeConverter<JSValue> {
    static JSValue valueFromJs(JSContext *ctx, const JSValue &val) {
        return val;
    }
    static JSValue valueToJs(JSContext *ctx, const JSValue &val) { return val; }
};

/**
 * If a function returns a reference to an existing JSValue, we need to increase
 * the reference count before returning.
 */
template <> struct TypeConverter<JSValue &> {
    static JSValue valueFromJs(JSContext *ctx, const JSValue &val) {
        return val;
    }
    static JSValue valueToJs(JSContext *ctx, const JSValue &val) {
        return JS_DupValue(ctx, val);
    }
};

template <> struct TypeConverter<bool> {
    static bool valueFromJs(JSContext *ctx, JSValue val) {
        return JS_ToBool(ctx, val);
    }
    static JSValue valueToJs(JSContext *ctx, const bool &val) {
        return JS_NewBool(ctx, val);
    }
};

// signed integral type or enum
template <typename T>
struct TypeConverter<
    T, typename std::enable_if<std::is_enum<T>::value ||
                               (std::is_signed<T>::value &&
                                std::is_integral<T>::value)>::type> {
    static T valueFromJs(JSContext *ctx, JSValue val) {
        int32_t n;
        JS_ToInt32(ctx, &n, val);
        return static_cast<T>(n);
    }
    static JSValue valueToJs(JSContext *ctx, const T &val) {
        return JS_NewInt32(ctx, static_cast<int32_t>(val));
    }
};

// unsigned integral type
template <typename T>
struct TypeConverter<
    T, typename std::enable_if<std::is_unsigned<T>::value>::type> {
    static T valueFromJs(JSContext *ctx, JSValue val) {
        uint32_t n;
        JS_ToUint32(ctx, &n, val);
        return static_cast<T>(n);
    }
    static JSValue valueToJs(JSContext *ctx, const T &val) {
        return JS_NewUint32(ctx, static_cast<uint32_t>(val));
    }
};

// float type
template <typename T>
struct TypeConverter<
    T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
    static T valueFromJs(JSContext *ctx, JSValue val) {
        double n;
        JS_ToFloat64(ctx, &n, val);
        return n;
    }
    static JSValue valueToJs(JSContext *ctx, const T &val) {
        return JS_NewFloat64(ctx, static_cast<double>(val));
    }
};

// std::vector
template <typename T> struct TypeConverter<std::vector<T>> {
    static JSValue valueToJs(JSContext *ctx, const std::vector<T> &v) {
        JSValue arr = JS_NewArray(ctx);
        JSValue push = JS_GetPropertyStr(ctx, arr, "push");
        for (size_t i = 0; i < v.size(); ++i) {
            JSValue arg = TypeConverter<T>::valueToJs(ctx, v[i]);
            JSValue ret = JS_Call(ctx, push, arr, 1, &arg);
            JS_FreeValue(ctx, ret);
            JS_FreeValue(ctx, arg);
        }
        JS_FreeValue(ctx, push);
        return arr;
    }
    static std::vector<T> valueFromJs(JSContext *ctx, JSValue arr) {
        int len;
        JSValue length = JS_GetPropertyStr(ctx, arr, "length");
        JS_ToInt32(ctx, &len, length);
        std::vector<T> vec(len);
        for (int i = 0; i < len; ++i) {
            JSValue item = JS_GetPropertyUint32(ctx, arr, i);
            vec[i] = TypeConverter<T>::valueFromJs(ctx, item);
            JS_FreeValue(ctx, item);
        }
        JS_FreeValue(ctx, length);
        return vec;
    }
};
template <typename T> struct TypeConverter<std::vector<T> &> {
    static JSValue valueToJs(JSContext *ctx, const std::vector<T> &v) {
        return TypeConverter<std::vector<T>>::valueToJs(ctx, v);
    }
};

template <typename T0> struct ScriptValueFromPartial {
    using T = typename remove_all<T0>::type;

    static T valueFromPartial(JSContext *ctx, JSValue partial) {
        assert(ScriptClass<T>::generated());
        JSClassID _id;
        void *opaque = JS_GetAnyOpaque(partial, &_id);
        if (opaque) {
            return *static_cast<T *>(
                static_cast<ObjectHeader *>(opaque)->get());
        } else {
            T val;
            ScriptClass<T>::populateFromPartial(ctx, val, partial);
            return val;
        }
    }
};

template <typename T>
struct TypeConverter<std::shared_ptr<T>,
                     typename std::enable_if<std::is_base_of<
                         T, std::enable_shared_from_this<T>>::value>::type> {
    static std::shared_ptr<T> valueFromJs(JSContext *ctx, JSValue arr) {
        if (JS_IsUndefined(arr)) {
            return {};
        }
        return TypeConverter<T *>::valueFromJs(ctx, arr)->shared_from_this();
    }
};

// default implementation for reference types; we can't make these from JS
template <typename T>
struct TypeConverter<
    T &,
    std::enable_if_t<!has_reference_specialization<std::decay_t<T>>::value &&
                     !std::is_class_v<std::decay_t<T>>>> {
    static JSValue valueToJs(JSContext *ctx, const T &val) {
        return TypeConverter<std::decay_t<T>>::valueToJs(ctx, val);
    }
};

template <typename T>
struct TypeConverter<
    const T &,
    std::enable_if_t<!has_reference_specialization<std::decay_t<T>>::value &&
                     !std::is_class_v<std::decay_t<T>>>> {
    static JSValue valueToJs(JSContext *ctx, const T &val) {
        return TypeConverter<std::decay_t<T>>::valueToJs(ctx, val);
    }
    static T valueFromJs(JSContext *ctx, const JSValue &val) {
        return TypeConverter<std::decay_t<T>>::valueFromJs(ctx, val);
    }
};

// reference type to class
template <typename T>
struct TypeConverter<
    T &,
    std::enable_if_t<!has_reference_specialization<std::decay_t<T>>::value &&
                     std::is_class_v<std::decay_t<T>>>> {
    static JSValue valueToJs(JSContext *ctx, const T &val);
    static T &valueFromJs(JSContext *ctx, const JSValue &val);
};

// copy instance from reference
template <typename T>
struct TypeConverter<
    T, std::enable_if_t<!std::is_reference_v<T> &&
                        !has_reference_specialization<std::decay_t<T>>::value &&
                        std::is_class_v<std::decay_t<T>>>> {
    static JSValue valueToJs(JSContext *ctx, const T &val) {
        return TypeConverter<T &>::valueToJs(ctx, val);
    }
    static T valueFromJs(JSContext *ctx, const JSValue &val) {
        return TypeConverter<T &>::valueFromJs(ctx, val);
    }
};

// default implementation for pointers which have a reference implementation
template <typename T>
struct TypeConverter<
    T *, std::enable_if_t<convertible_from_js<std::decay_t<T> &>::value>> {
    static JSValue valueToJs(JSContext *ctx, T *val) {
        return TypeConverter<T &>::valueToJs(ctx, *val);
    }
    static T *valueFromJs(JSContext *ctx, const JSValue &val) {
        return &TypeConverter<T &>::valueFromJs(ctx, val);
    }
};

// default implementation for pointers; we can't make these from JS
template <typename T>
struct TypeConverter<T *, typename std::enable_if<
                              !convertible_from_js<std::decay_t<T> &>::value>> {
    static JSValue valueToJs(JSContext *ctx, T *val) {
        return &TypeConverter<T &>::valueToJs(ctx, *val);
    }
};

// c string to JS value; can't convert from a JS string, because we would leak
// it
template <> struct TypeConverter<const char *> {
    static JSValue valueToJs(JSContext *ctx, const char *val) {
        return JS_NewString(ctx, val);
    }
};

// std::string
template <> struct TypeConverter<std::string> {
    static JSValue valueToJs(JSContext *ctx, const std::string &val) {
        return JS_NewStringLen(ctx, val.c_str(), val.size());
    }
    static std::string valueFromJs(JSContext *ctx, JSValue val) {
        const char *s = JS_ToCString(ctx, val);
        std::string result(s);
        JS_FreeCString(ctx, s);
        return result;
    }
};
template <> struct TypeConverter<std::string &> {
    static JSValue valueToJs(JSContext *ctx, const std::string &val) {
        return TypeConverter<std::string>::valueToJs(ctx, val);
    }
};
template <> struct TypeConverter<const std::string &> {
    static JSValue valueToJs(JSContext *ctx, const std::string &val) {
        return TypeConverter<std::string>::valueToJs(ctx, val);
    }
    static std::string valueFromJs(JSContext *ctx, JSValue val) {
        return TypeConverter<std::string>::valueFromJs(ctx, val);
    }
};

template <> struct TypeConverter<std::filesystem::path> {
    static JSValue valueToJs(JSContext *ctx, const std::filesystem::path &val) {
        return JS_NewString(ctx, (const char *)val.c_str());
    }
    static std::filesystem::path valueFromJs(JSContext *ctx, JSValue val) {
        const char *s = JS_ToCString(ctx, val);
        std::string result(s);
        JS_FreeCString(ctx, s);
        return result;
    }
};
template <> struct TypeConverter<std::filesystem::path &> {
    static JSValue valueToJs(JSContext *ctx, const std::filesystem::path &val) {
        return TypeConverter<std::filesystem::path>::valueToJs(ctx, val);
    }
};
template <> struct TypeConverter<const std::filesystem::path &> {
    static JSValue valueToJs(JSContext *ctx, const std::filesystem::path &val) {
        return TypeConverter<std::filesystem::path>::valueToJs(ctx, val);
    }
    static std::filesystem::path valueFromJs(JSContext *ctx, JSValue val) {
        return TypeConverter<std::filesystem::path>::valueFromJs(ctx, val);
    }
};

// std::string_view; can't convert from JS, because we would leak the string
template <> struct TypeConverter<std::string_view> {
    static JSValue valueToJs(JSContext *ctx, const std::string_view &val) {
        return JS_NewStringLen(ctx, val.data(), val.size());
    }
};

// optional
template <typename T> struct TypeConverter<std::optional<T>> {
    static JSValue valueToJs(JSContext *ctx, std::optional<T> v) {
        return v ? TypeConverter<T>::valueToJs(ctx, *v) : JS_UNDEFINED;
    }
    static std::optional<T> valueFromJs(JSContext *ctx, JSValue val) {
        if (JS_IsUndefined(val)) {
            return std::nullopt;
        }
        return std::optional<T>{TypeConverter<T>::valueFromJs(ctx, val)};
    }
};

}

// make sure we include the ScriptEngine-dependent specializations as well
#include "krit/script/ScriptEngine.h"
