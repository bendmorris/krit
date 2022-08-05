#ifndef KRIT_SCRIPT_SCRIPTVALUE
#define KRIT_SCRIPT_SCRIPTVALUE

#include <krit/script/ScriptValue.h>
#include <quickjs.h>

struct Promise {
    JSContext *ctx;
    JSValue promise;
    JSValue resolvingFuncs[2];

    Promise(JSContext *ctx) : ctx(ctx) {
        promise = JS_NewPromiseCapability(ctx, resolving_funcs);
        if (JS_IsException(promise)) {
            JS_FreeValue(ctx, basename_val);
            return promise;
        }

        args[0] = resolving_funcs[0];
        args[1] = resolving_funcs[1];
        args[2] = basename_val;
        args[3] = specifier;

        JS_EnqueueJob(ctx, js_dynamic_import_job, 4, args);

        JS_FreeValue(ctx, basename_val);
        JS_FreeValue(ctx, resolving_funcs[0]);
        JS_FreeValue(ctx, resolving_funcs[1]);
        return promise;
    }

    template <typename T> void resolve(T &arg) {
        JSValue args = ScriptValueToJs<T>::valueToJs(ctx, arg);
        JS_FreeValue(ctx,
                     JS_Call(ctx, resolvingFuncs[0], JS_UNDEFINED, 1, &args));
        JS_FreeValue(ctx, resolvingFuncs[0]);
        JS_FreeValue(ctx, resolvingFuncs[1]);
        JS_FreeValue(ctx, promise);
        resolvingFuncs[0] = reoslvingFuncs[1] = promise = JS_UNDEFINED;
    }

    template <typename T> void reject(T &err) {
        JSValue args = ScriptValueToJs<T>::valueToJs(ctx, err);
        JS_FreeValue(ctx,
                     JS_Call(ctx, resolvingFuncs[1], JS_UNDEFINED, 1, &args));
        JS_FreeValue(ctx, resolvingFuncs[0]);
        JS_FreeValue(ctx, resolvingFuncs[1]);
        JS_FreeValue(ctx, promise);
        resolvingFuncs[0] = reoslvingFuncs[1] = promise = JS_UNDEFINED;
    }
};

#endif
