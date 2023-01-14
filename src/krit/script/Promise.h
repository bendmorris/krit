#ifndef KRIT_SCRIPT_PROMISE
#define KRIT_SCRIPT_PROMISE

#include <krit/script/ScriptValue.h>
#include <quickjs.h>

namespace krit {

struct Promise {
    JSContext *ctx;
    JSValue promise;
    JSValue resolvingFuncs[2];

    Promise(JSContext *ctx) : ctx(ctx) {
        promise = JS_NewPromiseCapability(ctx, resolvingFuncs);
    }

    Promise(const Promise &p) {
        this->ctx = p.ctx;
        this->promise = JS_DupValue(p.ctx, p.promise);
        this->resolvingFuncs[0] = JS_DupValue(p.ctx, p.resolvingFuncs[0]);
        this->resolvingFuncs[1] = JS_DupValue(p.ctx, p.resolvingFuncs[1]);
    }

    Promise(Promise &&p) {
        this->ctx = p.ctx;
        this->promise = p.promise;
        this->resolvingFuncs[0] = p.resolvingFuncs[0];
        this->resolvingFuncs[1] = p.resolvingFuncs[1];
        p.promise = p.resolvingFuncs[0] = p.resolvingFuncs[1] = JS_UNDEFINED;
    }

    ~Promise() {
        JS_FreeValue(ctx, resolvingFuncs[0]);
        JS_FreeValue(ctx, resolvingFuncs[1]);
        JS_FreeValue(ctx, promise);
    }

    template <typename T> void resolve(T arg) const {
        JSValue args = ScriptValueToJs<T>::valueToJs(ctx, arg);
        JS_FreeValue(ctx,
                     JS_Call(ctx, resolvingFuncs[0], JS_UNDEFINED, 1, &args));
    }

    template <typename T> void reject(T err) const {
        JSValue args = ScriptValueToJs<T>::valueToJs(ctx, err);
        JS_FreeValue(ctx,
                     JS_Call(ctx, resolvingFuncs[1], JS_UNDEFINED, 1, &args));
    }
};

template <> struct ScriptValueToJs<Promise> {
    static JSValue valueToJs(JSContext *ctx, const Promise &v) {
        return JS_DupValue(ctx, v.promise);
    }
};

}

#endif
