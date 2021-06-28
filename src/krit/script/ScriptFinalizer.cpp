#include <vector>

#include "krit/script/ScriptEngine.h"
#include "krit/script/ScriptFinalizer.h"
#include "krit/utils/Panic.h"
#include "quickjs.h"

namespace krit {

enum ScriptClass : int;

void ScriptFinalizer::init(ScriptEngine *engine) {
    JSContext *ctx = engine->ctx;

    JSValue globalObj = JS_GetGlobalObject(ctx);
    JSValue symbol = JS_GetPropertyStr(ctx, globalObj, "Symbol");
    JSValue finalizerName = JS_NewString(ctx, "__finalizer");
    JSValue finalizerSymbol =
        JS_Call(ctx, symbol, JS_UNDEFINED, 1, &finalizerName);
    JS_SetPropertyStr(ctx, globalObj, "__finalizerSymbol", finalizerSymbol);
    JS_FreeValue(ctx, finalizerName);
    JS_FreeValue(ctx, symbol);
    JS_FreeValue(ctx, globalObj);
}

void ScriptEngine::addFinalizer(JSValue obj, ScriptClass classId) {
    JSValue finalizer = JS_NewObjectClass(ctx, classIds[classId + 1]);
    void *opaque = JS_GetOpaque(obj, classIds[classId]);
    if (!opaque) {
        panic("error: trying to create finalizer for null pointer; possible "
              "invalid class ID (%i)\n",
              (int)classId);
    }
    JS_SetOpaque(finalizer, opaque);
    JS_SetProperty(ctx, obj, JS_ValueToAtom(ctx, finalizerSymbol), finalizer);
}

}
