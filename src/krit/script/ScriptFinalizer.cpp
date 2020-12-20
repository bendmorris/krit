#include "krit/script/ScriptEngine.h"
#include "krit/utils/Panic.h"

namespace krit {

static JSClassID finalizerId;

static void genericFinalizer(JSRuntime *rt, JSValue val) {
    void *ptr = JS_GetOpaque(val, finalizerId);
    if (ptr) {
        // free(ptr);
        JS_SetOpaque(val, nullptr);
    }
}

static JSClassDef classDef = {
    "Finalizer",
    .finalizer = genericFinalizer,
};

void ScriptFinalizer::init(ScriptEngine *engine) {
    static bool initialized;
    if (!initialized) {
        JS_NewClassID(&finalizerId);
        JS_NewClass(ScriptEngine::rt, finalizerId, &classDef);
    }

    JSContext *ctx = engine->ctx;

    JSValue globalObj = JS_GetGlobalObject(ctx);
    JSValue symbol = JS_GetPropertyStr(ctx, globalObj, "Symbol");
    JSValue finalizerName = JS_NewString(ctx, "__finalizer");
    JSValue finalizerSymbol = JS_Call(ctx, symbol, JS_UNDEFINED, 1, &finalizerName);
    JS_SetPropertyStr(ctx, globalObj, "__finalizerSymbol", finalizerSymbol);
    JS_FreeValue(ctx, finalizerName);
    JS_FreeValue(ctx, symbol);
    JS_FreeValue(ctx, globalObj);

    JSValue proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, proto, nullptr, 0);
    JS_SetClassProto(ctx, finalizerId, proto);
}

void ScriptEngine::addFinalizer(JSValue obj, ScriptClass classId) {
    JSValue finalizer = JS_NewObjectClass(ctx, finalizerId);
    void *opaque = JS_GetOpaque(obj, classIds[classId]);
    if (!opaque) {
        panic("error: trying to create finalizer for null pointer; possible invalid class ID (%i)\n", (int)classId);
    }
    JS_SetOpaque(finalizer, opaque);
    JS_SetProperty(ctx, obj, JS_ValueToAtom(ctx, finalizerSymbol), finalizer);
}

}
