#pragma once

#include "krit/script/ObjectHeader.h"
#include "krit/script/OwnedValue.h"
#include "krit/script/ScriptValue.h"
#include "krit/utils/Panic.h"
#include "quickjs.h"
#include <cstring>
#include <iosfwd>
#include <list>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace krit {

template <class T, std::size_t = sizeof(T)>
std::true_type is_complete_impl(T *);
std::false_type is_complete_impl(...);

struct hash_instance_pair {
    template <class T1, class T2>
    size_t operator()(const std::pair<T1, T2> &p) const {
        // it's cheaper to just let multiple values for the same pointer land in
        // the same bucket than always calculate and xor two hashes in an
        // attempt to avoid collisions
        return std::hash<T2>{}(p.second);
    }
};

template <class T>
using is_complete = decltype(is_complete_impl(std::declval<T *>()));

#define JS_FUNC(n)                                                             \
    JSValue js_##n(JSContext *ctx, JSValueConst this_val, int argc,            \
                   JSValueConst *argv)
#define JS_GLOBAL_FUNC(n, a)                                                   \
    JS_SetPropertyStr(ctx, globalObj, #n, JS_NewCFunction(ctx, js_##n, #n, a))
#define JS_GLOBAL_FUNC2(n, f, a)                                               \
    JS_SetPropertyStr(ctx, globalObj, #n, JS_NewCFunction(ctx, f, #n, a))
#define JS_METHOD(obj, n, a)                                                   \
    JS_SetPropertyStr(ctx, obj, #n, JS_NewCFunction(ctx, js_##n, #n, a))
#define GET_ENGINE                                                             \
    ScriptEngine *engine =                                                     \
        static_cast<ScriptEngine *>(JS_GetContextOpaque(ctx));

template <typename Head>
void _unpackCallArgs(JSContext *ctx, JSValue *args, Head &head) {
    args[0] = TypeConverter<Head>::valueToJs(ctx, head);
}
template <typename Head, typename... Tail>
void _unpackCallArgs(JSContext *ctx, JSValue *args, Head &head, Tail &...tail) {
    _unpackCallArgs<Head>(ctx, args, head);
    _unpackCallArgs<Tail...>(ctx, &args[1], tail...);
}

struct DelayRequest {
    float duration;
    JSValue resolve;
    JSValue reject;
};

/**
 * ScriptEngine currently assumes the use of QuickJS.
 *
 * There should be one ScriptEngine instance per JS context. After constructing
 * an instance, initialize any special bridge functions or objects.
 *
 * Evaluate a JS file with `eval` and call functions using the various `call`
 * methods.
 */
struct ScriptEngine {
    static std::optional<std::vector<void (*)(ScriptEngine *)>>
        scriptClassInitializers;
    static void pushInitializer(void (*initializer)(ScriptEngine *)) {
        if (!scriptClassInitializers) {
            scriptClassInitializers = std::vector<void (*)(ScriptEngine *)>();
        }
        scriptClassInitializers->push_back(initializer);
    }

    static void baseFinalizer(JSRuntime *rt, JSValue val);
    static std::string serializeValue(JSContext *ctx, JSValue val);

    template <typename I>
    static std::string n2hexstr(I w, size_t hex_len = sizeof(I) << 1) {
        static const char *digits = "0123456789abcdef";
        std::string rc(hex_len, '0');
        for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
            rc[i] = digits[(w >> j) & 0x0f];
        return rc;
    }

    JSRuntime *rt;
    JSContext *ctx = nullptr;
    JSValue exports = JS_UNDEFINED;
    void *userData;
    JSValue features = JS_UNDEFINED;
    std::unordered_map<std::pair<int, const void *>, OwnedValue,
                       hash_instance_pair>
        instances;

    ScriptEngine();
    ~ScriptEngine();

    void eval(const std::string &scriptName, const std::string &src) {
        this->eval(scriptName.c_str(), src.c_str(), src.length());
    }
    void eval(const char *scriptName, const char *src, size_t len);
    std::string evalToString(const std::string &scriptName,
                             const std::string &src) {
        return this->evalToString(scriptName, src.c_str(), src.length());
    }
    std::string evalToString(const std::string &scriptName, const char *src,
                             size_t len);

    template <typename ReturnValue>
    void callPut(ReturnValue &dest, JSValue func) {
        JSValue jsResult = JS_Call(ctx, func, JS_UNDEFINED, 0, nullptr);
        checkForErrors();
        TypeConverter<ReturnValue>::valueFromJs(ctx, dest, jsResult);
        JS_FreeValue(ctx, jsResult);
        update();
    }
    template <typename ReturnValue, typename Arg, typename... ArgTypes>
    void callPut(ReturnValue &dest, JSValue func, Arg &arg, ArgTypes &...args) {
        JSValue jsArgs[1 + sizeof...(ArgTypes)];
        _unpackCallArgs<Arg, ArgTypes...>(this->ctx, jsArgs, arg, args...);

        JSValue jsResult =
            JS_Call(ctx, func, JS_UNDEFINED, 1 + sizeof...(ArgTypes), jsArgs);
        checkForErrors();
        TypeConverter<ReturnValue>::valueFromJs(ctx, dest, jsResult);
        JS_FreeValue(ctx, jsResult);
        for (long unsigned int i = 0; i < 1 + sizeof...(ArgTypes); ++i) {
            JS_FreeValue(ctx, jsArgs[i]);
        }
        update();
    }
    template <typename ReturnValue, typename... ArgTypes>
    void callPut(const char *functionName, ReturnValue &dest,
                 ArgTypes &...args) {
        JSValue func = JS_GetPropertyStr(ctx, exports, functionName);
        this->callPut<ReturnValue, ArgTypes...>(func, dest, args...);
        JS_FreeValue(ctx, func);
    }
    template <typename ReturnValue, typename... ArgTypes>
    void callPut(const std::string &functionName, ReturnValue &dest,
                 ArgTypes &...args) {
        return this->callPut<ReturnValue, ArgTypes...>(functionName.c_str(),
                                                       dest, args...);
    }

    template <typename ReturnValue> ReturnValue callReturn(JSValue func) {
        JSValue jsResult = JS_Call(ctx, func, JS_UNDEFINED, 0, nullptr);
        checkForErrors();
        ReturnValue dest;
        TypeConverter<ReturnValue>::valueFromJs(ctx, dest, jsResult);
        JS_FreeValue(ctx, jsResult);
        update();
        return dest;
    }
    template <typename ReturnValue, typename... ArgTypes>
    ReturnValue callReturn(JSValue func, ArgTypes &...args) {
        JSValue jsArgs[sizeof...(ArgTypes)];
        _unpackCallArgs<ArgTypes...>(this->ctx, jsArgs, args...);
        JSValue jsResult =
            JS_Call(ctx, func, JS_UNDEFINED, sizeof...(ArgTypes), jsArgs);
        checkForErrors();
        ReturnValue dest;
        TypeConverter<ReturnValue>::valueFromJs(ctx, dest, jsResult);
        JS_FreeValue(ctx, jsResult);
        for (long unsigned int i = 0; i < sizeof...(ArgTypes); ++i) {
            JS_FreeValue(ctx, jsArgs[i]);
        }
        update();
        return dest;
    }
    template <typename ReturnValue, typename... ArgTypes>
    ReturnValue callReturn(const char *functionName, ArgTypes &...args) {
        JSValue func = JS_GetPropertyStr(ctx, exports, functionName);
        ReturnValue result =
            this->callReturn<ReturnValue, ArgTypes...>(func, args...);
        JS_FreeValue(ctx, func);
        return result;
    }
    template <typename ReturnValue, typename... ArgTypes>
    ReturnValue callReturn(const std::string &functionName, ArgTypes &...args) {
        return this->callReturn<ReturnValue, ArgTypes...>(functionName.c_str(),
                                                          args...);
    }

    void callVoid(JSValue func) {
        JSValue jsResult = JS_Call(ctx, func, JS_UNDEFINED, 0, nullptr);
        checkForErrors();
        JS_FreeValue(ctx, jsResult);
        update();
    }
    template <typename... ArgTypes>
    void callVoid(JSValue func, ArgTypes &...args) {
        JSValue jsArgs[sizeof...(ArgTypes)];
        _unpackCallArgs<ArgTypes...>(this->ctx, jsArgs, args...);
        JSValue jsResult =
            JS_Call(ctx, func, JS_UNDEFINED, sizeof...(ArgTypes), jsArgs);
        checkForErrors();
        JS_FreeValue(ctx, jsResult);
        for (long unsigned int i = 0; i < sizeof...(ArgTypes); ++i) {
            JS_FreeValue(ctx, jsArgs[i]);
        }
        update();
    }
    template <typename... ArgTypes>
    void callVoid(const char *functionName, ArgTypes &...args) {
        JSValue func = JS_GetPropertyStr(ctx, exports, functionName);
        this->callVoid<ArgTypes...>(func, args...);
        JS_FreeValue(ctx, func);
    }
    template <typename... ArgTypes>
    void callVoid(const std::string &functionName, ArgTypes &...args) {
        this->callVoid<ArgTypes...>(functionName.c_str(), args...);
    }

    void update();
    void handleDelays(float elapsed);
    void checkForErrors();
    void checkForErrors(JSValue);

    template <typename T> JSValue create(void *data) {
        if (!data) {
            return JS_NULL;
        }
        JSValue val = JS_NewObjectClass(ctx, ScriptClass<T>::classId());
        auto header = ObjectHeader::create();
        header->setRaw(data);
        JS_SetOpaque(val, header);
        return val;
    }

    template <typename T> JSValue createOwned(T *data) {
        if (!data) {
            return JS_NULL;
        }
        JSValue val = JS_NewObjectClass(ctx, ScriptClass<T>::classId());
        tagOwned<T>(val, std::unique_ptr<T>(data));
        return val;
    }

    template <typename T>
    void tagOwned(JSValue val, std::unique_ptr<T> &&data) {
        auto header = ObjectHeader::create();
        header->setUnique(std::move(data));
        JS_SetOpaque(val, header);
    }

    template <typename T> JSValue createShared(std::shared_ptr<T> data) {
        if (!data) {
            return JS_NULL;
        }
        JSValue val = JS_NewObjectClass(ctx, ScriptClass<T>::classId());
        tagShared(val, data);
        return val;
    }

    void tagShared(JSValue val, std::shared_ptr<void> data);

    template <typename T> T *data() { return static_cast<T *>(this->userData); }

    JSValue delay(float duration);

    void setAtNamespace(const char **ns, const char *name, JSValue val) {
        JSValue obj = JS_GetGlobalObject(ctx);
        while (ns[0]) {
            JSValue child = JS_GetPropertyStr(ctx, obj, ns[0]);
            if (JS_IsUndefined(child)) {
                child = JS_NewObject(ctx);
                JS_DupValue(ctx, child);
                JS_SetPropertyStr(ctx, obj, ns[0], child);
            }
            JS_FreeValue(ctx, obj);
            obj = child;
            ++ns;
        }
        JS_SetPropertyStr(ctx, obj, name, val);
        JS_FreeValue(ctx, obj);
    }

    void dumpBacktrace(FILE *);

    std::string serializeValue(JSValue val);

    JSValue getCachedInstance(int classId, const void *p);
    void setCachedInstance(int classId, const void *p, JSValue val);

private:
    std::vector<DelayRequest> delayPromises;
};

template <typename T> struct TypeConverter<std::unique_ptr<T>> {
    static JSValue valueToJs(JSContext *ctx, std::unique_ptr<T> &&val) {
        if (!val) {
            return JS_NULL;
        }
        ScriptEngine *engine =
            static_cast<ScriptEngine *>(JS_GetContextOpaque(ctx));
        return engine->createOwned<T>(val.release());
    }
};

template <typename T> struct TypeConverter<std::unique_ptr<T> &> {
    static JSValue valueToJs(JSContext *ctx, std::unique_ptr<T> &val) {
        return TypeConverter<T *>::valueToJs(ctx, val.get());
    }
};

template <typename T> struct TypeConverter<std::shared_ptr<T>> {
    static JSValue valueToJs(JSContext *ctx, const std::shared_ptr<T> &val) {
        if (!val) {
            return JS_NULL;
        }
        ScriptEngine *engine =
            static_cast<ScriptEngine *>(JS_GetContextOpaque(ctx));
        return engine->createShared<T>(val);
    }
    static std::shared_ptr<T> valueFromJs(JSContext *ctx, JSValue val) {
        if (JS_IsNull(val) || !JS_IsObject(val)) {
            return nullptr;
        }
        auto header = ObjectHeader::header(val);
        switch (header->type()) {
            case ObjectHeader::OwnershipType::Shared: {
                return header->getShared<T>();
            }
            default: {
                assert(false);
                return {};
            }
        }
    }
};

template <typename ReturnT, typename... Args>
struct TypeConverter<std::function<ReturnT(Args...)>> {
    using FnT = std::function<ReturnT(Args...)>;
    static FnT valueFromJs(JSContext *ctx, JSValue val) {
        OwnedValue ownedFn(ctx, val);
        return [=](Args... args) {
            GET_ENGINE;
            return engine->callReturn<ReturnT, Args...>(*ownedFn, args...);
        };
    }
};

template <typename... Args> struct TypeConverter<std::function<void(Args...)>> {
    using FnT = std::function<void(Args...)>;
    static FnT valueFromJs(JSContext *ctx, JSValue val) {
        OwnedValue ownedFn(ctx, val);
        return [=](Args... args) {
            GET_ENGINE;
            engine->callVoid<Args...>(*ownedFn, args...);
        };
    }
};

}
