#include "krit/script/ScriptEngine.h"
#include "krit/script/ScriptAllocator.h"
#include "krit/script/ScriptClass.h"
#include <cstring>
#include <memory>
#include <stdio.h>

namespace krit {

std::unique_ptr<std::vector<void (*)(ScriptEngine *)>> ScriptEngine::scriptClassInitializers;

static std::string js_serialize_obj(JSContext *ctx, JSValueConst val) {
    const char *str = JS_ToCString(ctx, val);
    if (str) {
        std::string result(str);
        JS_FreeCString(ctx, str);
        return result;
    } else {
        return "(atom: " +
               std::string(JS_AtomToCString(ctx, JS_ValueToAtom(ctx, val))) +
               ")";
    }
}

static std::string js_std_get_error(JSContext *ctx,
                                    JSValueConst exception_val) {
    JSValue val;
    if (JS_IsError(ctx, exception_val)) {
        // an "Error" is an object of the Error type
        val = JS_GetPropertyStr(ctx, exception_val, "stack");
        if (!JS_IsUndefined(val)) {
            return js_serialize_obj(ctx, exception_val) + "\n" +
                   js_serialize_obj(ctx, val);
        }
        return "[Error]";
    } else if (JS_IsException(exception_val)) {
        // ...while an "Exception" is a native error type, and we must call
        // JS_GetException to find the actual error object
        JSValue err = JS_GetException(ctx);
        return js_std_get_error(ctx, err);
    } else {
        return js_serialize_obj(ctx, exception_val);
    }
}

static void js_std_dump_error(JSContext *ctx, JSValueConst exception_val) {
    std::string err = js_std_get_error(ctx, exception_val);
    fprintf(stderr, "%s\n", err.c_str());
}

static void js_std_promise_rejection_tracker(JSContext *ctx,
                                             JSValueConst promise,
                                             JSValueConst reason,
                                             int is_handled, void *opaque) {
    if (!is_handled) {
        fprintf(stderr, "Possibly unhandled promise rejection: ");
        js_std_dump_error(ctx, reason);
    }
}

static JSMallocFunctions allocFunctions{
    .js_malloc = ScriptAllocator::alloc,
    .js_free = ScriptAllocator::free,
    .js_realloc = ScriptAllocator::realloc,
};

ScriptEngine::ScriptEngine() {
    rt = JS_NewRuntime2(&allocFunctions, this);
    // rt = JS_NewRuntime();
    // JS_SetRuntimeOpaque(rt, this);
    JS_SetHostPromiseRejectionTracker(rt, js_std_promise_rejection_tracker,
                                      NULL);
    // initScriptClass<0>(*this);

    JS_SetMaxStackSize(rt, 4 * 1024 * 1024);
    JS_SetRuntimeOpaque(rt, this);

    ctx = JS_NewContext(rt);
    JS_SetContextOpaque(ctx, this);
    exports = JS_NewObject(ctx);
    JSValue globalObj = JS_GetGlobalObject(ctx);

    JS_SetPropertyStr(ctx, globalObj, "exports", JS_DupValue(ctx, exports));

    // finalizers
    JSValue symbol = JS_GetPropertyStr(ctx, globalObj, "Symbol");
    JSValue finalizerName = JS_NewString(ctx, "__finalizer");
    finalizerSymbol = JS_Call(ctx, symbol, JS_UNDEFINED, 1, &finalizerName);
    JS_SetPropertyStr(ctx, globalObj, "__finalizerSymbol",
                      JS_DupValue(ctx, finalizerSymbol));
    JS_FreeValue(ctx, finalizerName);
    JS_FreeValue(ctx, symbol);
    JS_FreeValue(ctx, globalObj);

    if (scriptClassInitializers) {
        for (auto &init : *scriptClassInitializers) {
            init(this);
        }
    }
}

ScriptEngine::~ScriptEngine() {
    JS_FreeValue(ctx, exports);
    JS_FreeValue(ctx, finalizerSymbol);
    JS_FreeContext(ctx);
    JS_RunGC(rt);
    // this will fail an assertion if values are still referenced
    JS_FreeRuntime(rt);
}

void ScriptEngine::eval(const char *scriptName, const char *src, size_t len) {
    JSValue result = JS_Eval(ctx, src, len, scriptName, JS_EVAL_TYPE_MODULE);
    if (JS_IsException(result) || JS_IsError(ctx, result)) {
        printf("error evaluating script: %s\n", scriptName);
        js_std_dump_error(ctx, result);
    }
    JS_FreeValue(ctx, result);
}

std::string ScriptEngine::evalToString(const std::string &scriptName,
                                       const char *src, size_t len) {
    JSValue result =
        JS_Eval(ctx, src, len, scriptName.c_str(), JS_EVAL_TYPE_GLOBAL);
    std::string s = js_std_get_error(ctx, result);
    JS_FreeValue(ctx, result);
    return s;
}

void ScriptEngine::update() {
    JSContext *c = ctx;
    int ret;
    while ((ret = JS_ExecutePendingJob(rt, &c))) {
        if (ret < 0) {
            checkForErrors();
        }
    }
}

void ScriptEngine::checkForErrors() {
    JSValue exception_val = JS_GetException(ctx);
    checkForErrors(exception_val);
    JS_FreeValue(ctx, exception_val);
}

void ScriptEngine::checkForErrors(JSValue exception_val) {
    if (JS_IsError(ctx, exception_val) || JS_IsException(exception_val)) {
        js_std_dump_error(ctx, exception_val);
    }
}

JSValue ScriptEngine::delay(float duration) {
    // create the promise; retain the `then` function
    JSValue resolvingFuncs[2];
    JSValue promise = JS_NewPromiseCapability(ctx, resolvingFuncs);

    // insert into our tracked promises
    bool inserted = false;
    if (!delayPromises.empty()) {
        for (size_t i = delayPromises.size(); i >= 0; --i) {
            auto it = delayPromises.begin() + i;
            if (it->duration <= 0)
                continue;
            if (duration > it->duration) {
                it->duration -= duration;
                this->delayPromises.emplace(
                    it + 1, DelayRequest{
                                .duration = duration,
                                .resolve = JS_DupValue(ctx, resolvingFuncs[0]),
                                .reject = JS_DupValue(ctx, resolvingFuncs[1])});
                inserted = true;
                break;
            } else {
                duration -= it->duration;
            }
        }
        if (!inserted) {
            this->delayPromises.emplace_back(
                DelayRequest{.duration = duration,
                             .resolve = JS_DupValue(ctx, resolvingFuncs[0]),
                             .reject = JS_DupValue(ctx, resolvingFuncs[1])});
        }
    }

    return promise;
}

void ScriptEngine::handleDelays(float elapsed) {
    if (!this->delayPromises.empty()) {
        this->delayPromises.front().duration -= elapsed;
        while (!this->delayPromises.empty() &&
               this->delayPromises.back().duration <= 0) {
            // complete this delay
            JSValue resolve = this->delayPromises.back().resolve;
            JSValue reject = this->delayPromises.back().reject;
            JS_FreeValue(this->ctx,
                         JS_Call(this->ctx, resolve, JS_UNDEFINED, 0, nullptr));
            JS_FreeValue(this->ctx, resolve);
            JS_FreeValue(this->ctx, reject);
            this->delayPromises.pop_back();
        }
    }
}

}
