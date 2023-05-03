#ifndef KRIT_SCRIPT_SCRIPTENGINE
#define KRIT_SCRIPT_SCRIPTENGINE

#include "krit/script/ScriptFinalizer.h"
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
        // it's cheaper to just let multiple values for the same pointer land in the same bucket
        // than always calculate and xor two hashes in an attempt to avoid collisions
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
    args[0] = ScriptValueToJs<Head>::valueToJs(ctx, head);
}
template <typename Head, typename... Tail>
void _unpackCallArgs(JSContext *ctx, JSValue *args, Head &head,
                     Tail &... tail) {
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
    static std::unique_ptr<std::vector<void (*)(ScriptEngine *)>>
        scriptClassInitializers;
    static void pushInitializer(void (*initializer)(ScriptEngine *)) {
        if (!scriptClassInitializers) {
            scriptClassInitializers =
                std::unique_ptr<std::vector<void (*)(ScriptEngine *)>>(
                    new std::vector<void (*)(ScriptEngine *)>());
        }
        scriptClassInitializers->push_back(initializer);
    }

    static void baseFinalizer(JSRuntime *rt, JSValue val);

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
    JSValue finalizerSymbol = JS_UNDEFINED;
    JSClassID finalizerId = 0;
    std::unordered_map<std::pair<int, const void *>, JSValue, hash_instance_pair>
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
        ScriptValueFromJs<ReturnValue>::valueFromJs(ctx, dest, jsResult);
        JS_FreeValue(ctx, jsResult);
        update();
    }
    template <typename ReturnValue, typename Arg, typename... ArgTypes>
    void callPut(ReturnValue &dest, JSValue func, Arg &arg,
                 ArgTypes &... args) {
        JSValue jsArgs[1 + sizeof...(ArgTypes)];
        _unpackCallArgs<Arg, ArgTypes...>(this->ctx, jsArgs, arg, args...);

        JSValue jsResult =
            JS_Call(ctx, func, JS_UNDEFINED, 1 + sizeof...(ArgTypes), jsArgs);
        checkForErrors();
        ScriptValueFromJs<ReturnValue>::valueFromJs(ctx, dest, jsResult);
        JS_FreeValue(ctx, jsResult);
        for (long unsigned int i = 0; i < 1 + sizeof...(ArgTypes); ++i) {
            JS_FreeValue(ctx, jsArgs[i]);
        }
        update();
    }
    template <typename ReturnValue, typename... ArgTypes>
    void callPut(const char *functionName, ReturnValue &dest,
                 ArgTypes &... args) {
        JSValue func = JS_GetPropertyStr(ctx, exports, functionName);
        this->callPut<ReturnValue, ArgTypes...>(func, dest, args...);
        JS_FreeValue(ctx, func);
    }
    template <typename ReturnValue, typename... ArgTypes>
    void callPut(const std::string &functionName, ReturnValue &dest,
                 ArgTypes &... args) {
        return this->callPut<ReturnValue, ArgTypes...>(functionName.c_str(),
                                                       dest, args...);
    }

    template <typename ReturnValue> ReturnValue callReturn(JSValue func) {
        JSValue jsResult = JS_Call(ctx, func, JS_UNDEFINED, 0, nullptr);
        checkForErrors();
        ReturnValue dest;
        ScriptValueFromJs<ReturnValue>::valueFromJs(ctx, dest, jsResult);
        JS_FreeValue(ctx, jsResult);
        update();
        return dest;
    }
    template <typename ReturnValue, typename Arg, typename... ArgTypes>
    ReturnValue callReturn(JSValue func, Arg &arg, ArgTypes &... args) {
        JSValue jsArgs[1 + sizeof...(ArgTypes)];
        _unpackCallArgs<Arg, ArgTypes...>(this->ctx, jsArgs, arg, args...);
        JSValue jsResult =
            JS_Call(ctx, func, JS_UNDEFINED, 1 + sizeof...(ArgTypes), jsArgs);
        checkForErrors();
        ReturnValue dest;
        ScriptValueFromJs<ReturnValue>::valueFromJs(ctx, dest, jsResult);
        JS_FreeValue(ctx, jsResult);
        for (long unsigned int i = 0; i < 1 + sizeof...(ArgTypes); ++i) {
            JS_FreeValue(ctx, jsArgs[i]);
        }
        update();
        return dest;
    }
    template <typename ReturnValue, typename... ArgTypes>
    ReturnValue callReturn(const char *functionName, ArgTypes &... args) {
        JSValue func = JS_GetPropertyStr(ctx, exports, functionName);
        ReturnValue result =
            this->callReturn<ReturnValue, ArgTypes...>(func, args...);
        JS_FreeValue(ctx, func);
        return result;
    }
    template <typename ReturnValue, typename... ArgTypes>
    ReturnValue callReturn(const std::string &functionName,
                           ArgTypes &... args) {
        return this->callReturn<ReturnValue, ArgTypes...>(functionName.c_str(),
                                                          args...);
    }

    void callVoid(JSValue func) {
        JSValue jsResult = JS_Call(ctx, func, JS_UNDEFINED, 0, nullptr);
        checkForErrors();
        JS_FreeValue(ctx, jsResult);
        update();
    }
    template <typename Arg, typename... ArgTypes>
    void callVoid(JSValue func, Arg &arg, ArgTypes &... args) {
        JSValue jsArgs[1 + sizeof...(ArgTypes)];
        _unpackCallArgs<Arg, ArgTypes...>(this->ctx, jsArgs, arg, args...);
        JSValue jsResult =
            JS_Call(ctx, func, JS_UNDEFINED, 1 + sizeof...(ArgTypes), jsArgs);
        checkForErrors();
        JS_FreeValue(ctx, jsResult);
        for (long unsigned int i = 0; i < 1 + sizeof...(ArgTypes); ++i) {
            JS_FreeValue(ctx, jsArgs[i]);
        }
        update();
    }
    template <typename... ArgTypes>
    void callVoid(const char *functionName, ArgTypes &... args) {
        JSValue func = JS_GetPropertyStr(ctx, exports, functionName);
        this->callVoid<ArgTypes...>(func, args...);
        JS_FreeValue(ctx, func);
    }
    template <typename... ArgTypes>
    void callVoid(const std::string &functionName, ArgTypes &... args) {
        this->callVoid<ArgTypes...>(functionName.c_str(), args...);
    }

    void update();
    void handleDelays(float elapsed);
    void checkForErrors();
    void checkForErrors(JSValue, FILE *f = stderr);

    template <typename T> JSValue create(void *data) {
        if (!data) {
            return JS_NULL;
        }
        JSValue val = JS_NewObjectClass(ctx, ScriptClass<T>::classId());
        JS_SetOpaque(val, data);
        return val;
    }

    template <typename T> JSValue createOwned(void *data) {
        if (!data) {
            return JS_NULL;
        }
        JSValue val = JS_NewObjectClass(ctx, ScriptClass<T>::classId());
        JS_SetOpaque(val, data);
        tagOwned<T>(val, data);
        return val;
    }

    template <typename T>
    void tagOwned(JSValue val, void *data, bool explicitDestruct = false) {
        JSValue finalizer =
            JS_NewObjectClassInline(ctx, finalizerId, sizeof(FinalizerData));
        FinalizerData *f =
            static_cast<FinalizerData *>(JS_GetOpaque(finalizer, 0));
        new (f) FinalizerData();
        if (explicitDestruct) {
            f->ownExplicitDestruct<T>(data);
        } else {
            f->own<T>(data);
        }
        JS_SetOpaque(finalizer, f);
        JS_SetProperty(ctx, val, JS_ValueToAtom(ctx, finalizerSymbol),
                       finalizer);
        JS_FreeValue(ctx, finalizerSymbol);
    }

    template <typename T> JSValue createShared(std::shared_ptr<T> data) {
        if (!data) {
            return JS_NULL;
        }
        JSValue val = JS_NewObjectClass(ctx, ScriptClass<T>::classId());
        JS_SetOpaque(val, data.get());
        tagShared(val, data);
        return val;
    }

    void tagShared(JSValue val, std::shared_ptr<void> data) {
        JSValue finalizer =
            JS_NewObjectClassInline(ctx, finalizerId, sizeof(FinalizerData));
        FinalizerData *f =
            static_cast<FinalizerData *>(JS_GetOpaque(finalizer, 0));
        new (f) FinalizerData();
        f->share(data);
        JS_SetOpaque(finalizer, f);
        JS_SetProperty(ctx, val, JS_ValueToAtom(ctx, finalizerSymbol),
                       finalizer);
        JS_FreeValue(ctx, finalizerSymbol);
    }

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

private:
    std::vector<DelayRequest> delayPromises;
};

template <typename T> struct ScriptValueToJs<std::unique_ptr<T>> {
    static JSValue valueToJs(JSContext *ctx, std::unique_ptr<T> &&val) {
        if (!val) {
            return JS_NULL;
        }
        ScriptEngine *engine =
            static_cast<ScriptEngine *>(JS_GetContextOpaque(ctx));
        return engine->createOwned<T>(val.release());
    }
};

template <typename T> struct ScriptValueToJs<std::unique_ptr<T> &> {
    static JSValue valueToJs(JSContext *ctx, std::unique_ptr<T> &val) {
        return ScriptValueToJs<T *>::valueToJs(ctx, val.get());
    }
};

template <typename T> struct ScriptValueFromJs<std::shared_ptr<T>> {
    static std::shared_ptr<T> valueFromJs(JSContext *ctx, JSValue val) {
        if (JS_IsNull(val) || !JS_IsObject(val)) {
            return nullptr;
        }
        ScriptEngine *engine =
            static_cast<ScriptEngine *>(JS_GetContextOpaque(ctx));
        JSValue finalizer = JS_GetProperty(
            ctx, val, JS_ValueToAtom(ctx, engine->finalizerSymbol));
        if (!JS_IsObject(finalizer)) {
            return nullptr;
        }
        FinalizerData *f = static_cast<FinalizerData *>(
            JS_GetOpaque(finalizer, engine->finalizerId));
        if (!f) {
            assert(false);
            return nullptr;
        }
        if (!std::holds_alternative<SharedData>(f->data)) {
            assert(false);
            return nullptr;
        }
        JS_FreeValue(ctx, finalizer);
        return std::static_pointer_cast<T>(std::get<SharedData>(f->data).p);
    }
};

template <typename T> struct ScriptValueToJs<std::shared_ptr<T>> {
    static JSValue valueToJs(JSContext *ctx, const std::shared_ptr<T> &val) {
        if (!val) {
            return JS_NULL;
        }
        ScriptEngine *engine =
            static_cast<ScriptEngine *>(JS_GetContextOpaque(ctx));
        return engine->createShared<T>(val);
    }
};

}

#endif
