#include "CrashHandler.h"
#include "krit/Engine.h"
#include <csignal>
#include <cstdlib>
#ifdef KRIT_BACKWARD
#include "backward.hpp"
#endif

namespace krit {

static void readFileToStderr(FILE *f) {
    char s[1024];
    size_t read = 0;
    while ((read = fread(s, 1, 1024, f))) {
        fwrite(s, 1, read, stderr);
    }
}

static void sigintHandler(int) {
    if (engine) {
        engine->quit();
    } else {
        exit(0);
    }
}

static void errorHandler(const char *fmt, ...) {
    static FILE *errorLog = nullptr;
    static bool inFallback = false;
    va_list args;
    va_start(args, fmt);
    if (inFallback) {
        CrashHandler::exit(3);
    } else if (errorLog) {
        inFallback = true;
        fputs("hit a subsequent error during the error handler; aborting\n",
              errorLog);
        fprintf(errorLog, fmt, args);
        fputs("\n", errorLog);
        if (errorLog != stderr) {
            readFileToStderr(errorLog);
        }
        CrashHandler::exit(2);
    }
    errorLog = fopen("crash.log", "w");
    vfprintf(errorLog, fmt, args);
    fputs("\n", errorLog);
    fflush(errorLog);
    // auto trace = std::basic_stacktrace::current().to_string();
    // fputs(trace.c_str());
    // fflush(errorLog);
#ifdef KRIT_BACKWARD
    {
        backward::StackTrace st;
        st.load_here(32);

        backward::TraceResolver tr;
        tr.load_stacktrace(st);
        for (size_t i = 0; i < st.size(); ++i) {
            backward::ResolvedTrace trace = tr.resolve(st[i]);
            fprintf(errorLog, "# %zu %s %s [%p]\n", i, trace.object_filename.c_str(), trace.object_function.c_str(), trace.addr);
        }
        fflush(errorLog);
    }
#endif
    if (engine && engine->script.ctx) {
        engine->script.dumpBacktrace(errorLog);
        if (!JS_IsUndefined(engine->scriptContext)) {
            JSValue stringified =
                JS_JSONStringify(engine->script.ctx, engine->scriptContext,
                                 JS_UNDEFINED, JS_UNDEFINED);
            const char *s = JS_ToCString(engine->script.ctx, stringified);
            if (s) {
                fprintf(errorLog, "Script context: %s\n", s);
            }
            JS_FreeCString(engine->script.ctx, s);
        }
    }
    fclose(errorLog);
    if (errorLog != stderr) {
        FILE *f = fopen("crash.log", "r");
        readFileToStderr(f);
        fclose(f);
    }
    CrashHandler::exit(1);
}

static void signalHandler(int sig) {
#ifdef KRIT_LINUX
    errorHandler("signal %i (%s)", sig, strsignal(sig));
#else
    errorHandler("signal %i", sig);
#endif
}

static void terminateHandler() {
    auto exc = std::current_exception();
    if (exc) {
        try {
            std::rethrow_exception(exc);
        } catch (const std::exception &e) {
            errorHandler("std::terminate: %s", e.what());
        } catch (const std::string &e) {
            errorHandler("std::terminate: %s", e.c_str());
        } catch (const char *e) {
            errorHandler("std::terminate: %s", e);
        } catch (...) {
            errorHandler("std::terminate (unknown exception)");
        }
    } else {
        errorHandler("std::terminate (no exception)");
    }
}

void CrashHandler::init() {
    static bool registeredHandlers = false;
    if (!registeredHandlers) {
#ifndef __EMSCRIPTEN
        registeredHandlers = true;
        std::signal(SIGINT, sigintHandler);
        std::signal(SIGSEGV, signalHandler);
        std::signal(SIGABRT, signalHandler);
        std::set_terminate(terminateHandler);
#endif
    }
}

void CrashHandler::exit(int code) {
    std::signal(SIGINT, SIG_DFL);
    std::signal(SIGSEGV, SIG_DFL);
    std::signal(SIGABRT, SIG_DFL);
    std::exit(code);
}

}
