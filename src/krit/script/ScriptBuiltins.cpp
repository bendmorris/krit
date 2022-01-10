#include "krit/App.h"
#include "krit/io/Io.h"
#include "krit/script/ScriptBridge.h"
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
        if (i) {
            putchar(' ');
        }
        str = JS_ToCStringLen(ctx, &len, argv[i]);
        if (!str) {
            return JS_EXCEPTION;
        }
        fwrite(str, 1, len, stdout);
        JS_FreeCString(ctx, str);
    }

    putchar('\n');
    return JS_UNDEFINED;
}

JS_FUNC(Log_setLogLevel) {
    int level;
    JS_ToInt32(ctx, &level, argv[0]);
    Log::level = (LogLevel)level;
    return JS_UNDEFINED;
}

#define DEFINE_LOG_METHOD(level)                                               \
    JS_FUNC(Log_##level) {                                                     \
        const char *s = JS_ToCString(ctx, argv[0]);                            \
        Log::level(s);                                                         \
        JS_FreeCString(ctx, s);                                                \
        return JS_UNDEFINED;                                                   \
    }
DEFINE_LOG_METHOD(debug)
DEFINE_LOG_METHOD(info)
DEFINE_LOG_METHOD(warn)
DEFINE_LOG_METHOD(error)
DEFINE_LOG_METHOD(fatal)
DEFINE_LOG_METHOD(success)
#undef DEFINE_LOG_METHOD

JS_FUNC(exit) {
    int code = 0;
    if (argc > 0) {
        JS_ToInt32(ctx, &code, argv[0]);
    }
    if (code) {
        exit(code);
    } else {
        App::ctx.app->quit();
    }
    return JS_UNDEFINED;
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
    int len;
    char *content =
        FileIo::read(ScriptValue<std::string>::jsToValue(ctx, argv[0]), &len);
    auto rt = JS_NewStringLen(ctx, content, len);
    FileIo::free(content);
    return rt;
}

}
