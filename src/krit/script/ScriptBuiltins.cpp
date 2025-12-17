#include "krit/Engine.h"
#include "krit/asset/TextureAtlas.h"
#include "krit/io/Io.h"
#include "krit/script/ScriptClass.h"
#include "krit/script/ScriptEngine.h"
#include "krit/utils/Log.h"
#include "quickjs.h"
#include <stdio.h>
#include <stdlib.h>

namespace krit {

JS_FUNC(console_log) {
    const char *str;
    size_t len;

    for (int i = 0; i < argc; i++) {
        str = JS_ToCStringLen(ctx, &len, argv[i]);
        if (!str) {
            return JS_EXCEPTION;
        }
        AREA_LOG_OUTPUT("script", i ? " %.*s" : "%.*s", static_cast<int>(len), str);
        JS_FreeCString(ctx, str);
    }

    return JS_UNDEFINED;
}

char buf[10*1024];
JS_FUNC(Log_addLogSink) {
    JSValue func = argv[0];
    engine->script.holdValue(func);
    Log::addLogSink([=](LogLevel level, std::string_view area, const char *fmt, va_list args) {
        size_t len = vsnprintf(buf, sizeof(buf), fmt, args);
        if (len >= sizeof(buf)) {
            // oh no
            len = sizeof(buf) - 1;
        }
        JSValue jsArea = JS_NewStringLen(ctx, area.data(), area.size());
        JSValue jsFmt = JS_NewStringLen(ctx, buf, len);
        JSValue jsLevel = JS_NewUint32(ctx, static_cast<uint32_t>(level));
        JSValue callArgs[3] { jsArea, jsFmt, jsLevel };
        JS_FreeValue(ctx, JS_Call(ctx, func, JS_UNDEFINED, 3, callArgs));
        JS_FreeValue(ctx, jsArea);
        JS_FreeValue(ctx, jsFmt);
        JS_FreeValue(ctx, jsLevel);
    });
    return JS_UNDEFINED;
}

JS_FUNC(Log_setLogLevel) {
    int level;
    JS_ToInt32(ctx, &level, argv[0]);
    Log::defaultLevel = (LogLevel)level;
    return JS_UNDEFINED;
}

#define DEFINE_LOG_METHOD(level)                                               \
    JS_FUNC(Log_##level) {                                                     \
        const char *s = JS_ToCString(ctx, argv[0]);                            \
        Log::level("script", s);                                               \
        JS_FreeCString(ctx, s);                                                \
        return JS_UNDEFINED;                                                   \
    }
DEFINE_LOG_METHOD(debug)
DEFINE_LOG_METHOD(info)
DEFINE_LOG_METHOD(warn)
DEFINE_LOG_METHOD(error)
DEFINE_LOG_METHOD(output)
DEFINE_LOG_METHOD(fatal)
#undef DEFINE_LOG_METHOD

JS_FUNC(exit) {
    int code = 0;
    if (argc > 0) {
        JS_ToInt32(ctx, &code, argv[0]);
    }
    if (code) {
        exit(code);
    } else {
        engine->quit();
    }
    return JS_UNDEFINED;
}

[[noreturn]] JS_FUNC(abort) {
    if (argc > 0) {
        const char *s = JS_ToCString(ctx, argv[0]);
        if (s) {
            panic(s);
        }
        JS_FreeCString(ctx, s);
    }
    abort();
}

JS_FUNC(__id) {
    if (JS_IsUndefined(argv[0]) || JS_IsNull(argv[0])) {
        return JS_NewInt32(ctx, 0);
    }
    return JS_GetPropertyStr(ctx, argv[0], "__id");
}

/**
 * Returns a Promise which resolves in a specific amount of time.
 */
JS_FUNC(timeout) {
    GET_ENGINE;
    double duration;
    JS_ToFloat64(ctx, &duration, argv[0]);
    return engine->delay(duration);
}

JS_FUNC(gc) {
    JSRuntime *rt = JS_GetRuntime(ctx);
    JS_RunGC(rt);
    return JS_UNDEFINED;
}

JS_FUNC(dumpMemoryUsage) {
    JSMemoryUsage mem;
    JSRuntime *rt = JS_GetRuntime(ctx);
    JS_ComputeMemoryUsage(rt, &mem);
    JS_DumpMemoryUsage(stdout, &mem, rt);
    return JS_UNDEFINED;
}

JS_FUNC(readFile) {
    std::string s = engine->io->readFile(
        ScriptValueFromJs<const char *>::valueFromJs(ctx, argv[0]));
    return ScriptValueToJs<std::string>::valueToJs(ctx, s);
}

JS_FUNC(getImage) {
    GET_ENGINE;
    const char *s = JS_ToCString(ctx, argv[0]);
    ImageRegion *img = new ImageRegion(krit::engine->getImage(s));
    JS_FreeCString(ctx, s);
    return engine->createOwned<ImageRegion>(img);
}

JS_FUNC(getAtlasRegion) {
    GET_ENGINE;
    const char *s = JS_ToCString(ctx, argv[0]);
    const char *s2 = JS_ToCString(ctx, argv[1]);
    ImageRegion *img =
        new ImageRegion(krit::engine->getAtlas(s)->getRegion(s2));
    JS_FreeCString(ctx, s2);
    JS_FreeCString(ctx, s);
    return engine->createOwned<ImageRegion>(img);
}

}