#include "krit/script/ScriptEngine.h"
#include "krit/script/ScriptClass.h"
#include "krit/script/ScriptBridge.h"
#include "krit/script/ScriptFinalizer.h"
#include <cstring>

namespace krit {

JSRuntime *ScriptEngine::rt = JS_NewRuntime();

static std::string js_serialize_obj(JSContext *ctx, JSValueConst val) {
    const char *str = JS_ToCString(ctx, val);
    if (str) {
        std::string result(str);
        JS_FreeCString(ctx, str);
        return result;
    } else {
        return "(atom: " + std::string(JS_AtomToCString(ctx, JS_ValueToAtom(ctx, val))) + ")";
    }
}

static std::string js_std_get_error(JSContext *ctx, JSValueConst exception_val) {
    JSValue val;
    if (JS_IsError(ctx, exception_val)) {
        // an "Error" is an object of the Error type
        val = JS_GetPropertyStr(ctx, exception_val, "stack");
        if (!JS_IsUndefined(val)) {
            return js_serialize_obj(ctx, exception_val) + "\n" + js_serialize_obj(ctx, val);
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

std::vector<JSClassID> ScriptEngine::classIds(ScriptClassCount);

template <int N> static void initScriptClasses() {
    JSRuntime *rt = ScriptEngine::rt;
    auto &classIds = ScriptEngine::classIds;
    JSClassDef *classDef = scriptClassDef<N>();
    JS_NewClassID(&classIds[N]);
    JS_NewClass(rt, classIds[N], classDef);
    initScriptClasses<N + 1>();
}

template <> void initScriptClasses<ScriptClassCount>() {}

template <int N> static void registerScriptClasses(ScriptEngine *engine) {
    JSContext *ctx = engine->ctx;
    JSClassDef *classDef = scriptClassDef<N>();
    auto funcs = scriptClassProtoFuncs<N>();
    JSValue proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, proto, funcs.first, funcs.second);
    JS_SetClassProto(ctx, engine->classIds[N], proto);
    JSValue globalObj = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, globalObj, classDef->class_name, JS_DupValue(ctx, proto));
    registerScriptClasses<N + 1>(engine);
}

template <> void registerScriptClasses<ScriptClassCount>(ScriptEngine *engine) {}

ScriptEngine::ScriptEngine() {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        initScriptClasses<0>();
    }

    ctx = JS_NewContext(rt);
    JS_SetContextOpaque(ctx, this);
    exports = JS_NewObject(ctx);
    JSValue globalObj = JS_GetGlobalObject(ctx);

    JS_SetPropertyStr(ctx, globalObj, "exports", exports);
    JS_FreeValue(ctx, globalObj);

    ScriptFinalizer::init(this);
    registerScriptClasses<0>(this);
    initScriptBridge(*this);
}


ScriptEngine::~ScriptEngine() {
    JS_FreeValue(ctx, exports);
    JS_FreeContext(ctx);
}

void ScriptEngine::eval(const char *scriptName, const char *src, size_t len) {
    JSValue result = JS_Eval(ctx, src, len, scriptName, JS_EVAL_TYPE_MODULE);
    if (JS_IsException(result) || JS_IsError(ctx, result)) {
        printf("error evaluating script: %s\n", scriptName);
        js_std_dump_error(ctx, result);
    }
    JS_FreeValue(ctx, result);
}

std::string ScriptEngine::evalToString(const std::string &scriptName, const char *src, size_t len) {
    JSValue result = JS_Eval(ctx, src, len, scriptName.c_str(), JS_EVAL_TYPE_GLOBAL);
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

JSValue ScriptEngine::create(ScriptClass e, void *data) {
    if (!data) {
        return JS_NULL;
    }
    JSValue val = JS_NewObjectClass(ctx, classIds[e]);
    JS_SetOpaque(val, data);
    return val;
}

JSValue ScriptEngine::createOwned(ScriptClass e, void *data) {
    if (!data) {
        return JS_NULL;
    }
    JSValue val = JS_NewObjectClass(ctx, classIds[e]);
    JS_SetOpaque(val, data);
    addFinalizer(val, e);
    return val;
}

JSValue ScriptEngine::delay(double duration) {
    // create the promise; retain the `then` function
    JSValue resolvingFuncs[2];
    JSValue promise = JS_NewPromiseCapability(ctx, resolvingFuncs);

    // insert into our tracked promises
    bool inserted = false;
    for (auto it = this->delayPromises.begin(); it != this->delayPromises.end(); ++it) {
        if (it->duration <= 0) continue;
        if (duration < it->duration) {
            it->duration -= duration;
            this->delayPromises.emplace(it, DelayRequest { .duration = duration, .resolve = JS_DupValue(ctx, resolvingFuncs[0]), .reject = JS_DupValue(ctx, resolvingFuncs[1]) });
            inserted = true;
            break;
        } else {
            duration -= it->duration;
        }
    }
    if (!inserted) {
        this->delayPromises.emplace_back(DelayRequest { .duration = duration, .resolve = JS_DupValue(ctx, resolvingFuncs[0]), .reject = JS_DupValue(ctx, resolvingFuncs[1]) });
    }

    return promise;
}

void ScriptEngine::update(UpdateContext &ctx) {
    if (!this->delayPromises.empty()) {
        this->delayPromises.front().duration -= ctx.elapsed;
        while (!this->delayPromises.empty() && this->delayPromises.front().duration <= 0) {
            // complete this delay
            JSValue resolve = this->delayPromises.front().resolve;
            JSValue reject = this->delayPromises.front().reject;
            JS_FreeValue(this->ctx, JS_Call(this->ctx, resolve, JS_UNDEFINED, 0, nullptr));
            JS_FreeValue(this->ctx, resolve);
            JS_FreeValue(this->ctx, reject);
            this->delayPromises.pop_front();
        }
    }
}

}
