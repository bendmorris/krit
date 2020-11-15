#include "krit/script/ScriptEngine.h"
#include <cstring>

namespace krit {

JSRuntime *ScriptEngine::rt = nullptr;

static void js_dump_obj(JSContext *ctx, FILE *f, JSValueConst val) {
    const char *str;

    str = JS_ToCString(ctx, val);
    if (str) {
        fprintf(f, "%s\n", str);
        JS_FreeCString(ctx, str);
    } else {
        fprintf(f, "[exception]\n");
    }
}

const char *js_serialize_obj(JSContext *ctx, JSValueConst val) {
    const char *str;
    str = JS_ToCString(ctx, val);
    if (str) {
        return str;
    } else {
        return nullptr;
    }
}

static void js_std_dump_error1(JSContext *ctx, JSValueConst exception_val) {
    JSValue val;

    js_dump_obj(ctx, stderr, exception_val);
    if (JS_IsError(ctx, exception_val)) {
        // an "Error" is an object of the Error type
        val = JS_GetPropertyStr(ctx, exception_val, "stack");
        if (!JS_IsUndefined(val)) {
            js_dump_obj(ctx, stderr, val);
        }
        JS_FreeValue(ctx, val);
    } else if (JS_IsException(exception_val)) {
        // while an "Exception" is a native error type, and we must call
        // JS_GetException to find the actual error object
        JSValue err = JS_GetException(ctx);
        js_std_dump_error1(ctx, err);
    }
}

ScriptEngine::ScriptEngine() {
    ctx = JS_NewContext(rt);
    JS_SetContextOpaque(ctx, this);
    exports = JS_NewObject(ctx);
    JSValue globalObj = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, globalObj, "exports", exports);
    JS_FreeValue(ctx, globalObj);
}

ScriptEngine::~ScriptEngine() {
    JS_FreeValue(ctx, exports);
    JS_FreeContext(ctx);
}

void ScriptEngine::eval(const char *scriptName, const char *src, size_t len) {
    JSValue result = JS_Eval(ctx, src, len, scriptName, JS_EVAL_TYPE_MODULE);
    if (JS_IsException(result) || JS_IsError(ctx, result)) {
        printf("error evaluating script: %s\n", scriptName);
        js_std_dump_error1(ctx, result);
    }
    JS_FreeValue(ctx, result);
}

char *ScriptEngine::evalToString(const std::string &scriptName, const char *src, size_t len) {
    JSValue result = JS_Eval(ctx, src, len, scriptName.c_str(), JS_EVAL_TYPE_GLOBAL);
    const char *serialized = js_serialize_obj(ctx, result);
    char *s;
    if (serialized) {
        s = new char[strlen(serialized) + 1];
        strcpy(s, serialized);
        JS_FreeCString(ctx, serialized);
    } else {
        s = new char[6];
        strcpy(s, "ERROR");
    }
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
        js_std_dump_error1(ctx, exception_val);
    }
}

}
