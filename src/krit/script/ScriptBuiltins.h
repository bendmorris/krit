#ifndef KRIT_SCRIPT_SCRIPTBUILTINS
#define KRIT_SCRIPT_SCRIPTBUILTINS

namespace krit {

JS_FUNC(console_log);
JS_FUNC(Log_addLogSink);
JS_FUNC(Log_setLogLevel);
#define DEFINE_LOG_METHOD(level) JS_FUNC(Log_##level);
DEFINE_LOG_METHOD(debug)
DEFINE_LOG_METHOD(info)
DEFINE_LOG_METHOD(warn)
DEFINE_LOG_METHOD(error)
DEFINE_LOG_METHOD(fatal)
DEFINE_LOG_METHOD(output)
#undef DEFINE_LOG_METHOD

JS_FUNC(exit);
JS_FUNC(abort);
JS_FUNC(__id);
JS_FUNC(timeout);
JS_FUNC(gc);
JS_FUNC(dumpMemoryUsage);
JS_FUNC(readFile);
JS_FUNC(getImage);
JS_FUNC(getAtlasRegion);
JS_FUNC(encodeString);
JS_FUNC(decodeString);

}

#endif
