#include "krit/script/ScriptEngine.h"
#include "krit/script/ScriptBridge.h"

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

JS_FUNC(process_exit) {
    int code = 0;
    if (argc > 0) {
        JS_ToInt32(ctx, &code, argv[0]);
    }
    exit(code);
}

}
