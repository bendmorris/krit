#define SDL_MAIN_HANDLED
#include "krit/Engine.h"
#include "krit/Options.h"
#include "krit/utils/Log.h"
#include "krit/utils/Panic.h"
#include "krit/utils/Signal.h"
#include <stdlib.h>
#include <string.h>
#include <vector>
#ifdef KRIT_DESKTOP
#include "argparse/argparse.hpp"
#endif

namespace krit {

struct RenderContext;

static std::unique_ptr<Engine> _engine;

void gameMain(int argc, char *argv[]) {
    KritOptions options;
    gameOptions(options);

#ifdef KRIT_DESKTOP
    argparse::ArgumentParser program(options.programName.c_str());
    program.add_argument("-v", "--verbose")
        .action([&](const auto &) {
            if (options.logLevel > LogLevel::Debug) {
                options.logLevel = static_cast<LogLevel>(
                    static_cast<int>(options.logLevel) - 1);
            }
        })
        .append()
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("increase the log level (default: ERR)");
    program.add_argument("-f", "--feature")
        .action([&](const std::string &s) { options.features.push_back(s); })
        .append()
        .help("set feature values");
    program.add_argument("-e", "--eval")
        .action([&](const std::string &s) { options.jsFiles.push_back(s); })
        .append()
        .help("evaluate files at startup");

    program.parse_args(argc, argv);

    Log::level = options.logLevel;
#endif

    _engine = std::unique_ptr<Engine>(new Engine(options));

    _engine->onBegin = [&]() {
        auto &engine = *_engine;
        auto &script = engine.script;
        auto &jsCtx = script.ctx;

        {
            JSValue global = JS_GetGlobalObject(jsCtx);
            JS_SetPropertyStr(
                jsCtx, global, "frame",
                ScriptValueToJs<UpdateContext *>::valueToJs(jsCtx, &frame));
            JS_SetPropertyStr(jsCtx, global, "render",
                              ScriptValueToJs<RenderContext *>::valueToJs(
                                  jsCtx, &engine.ctx));
            JS_FreeValue(jsCtx, global);
        }
        {
            // parse feature flags
            for (auto &f : options.features) {
                size_t eq = f.find('=');
                LOG_INFO("setting feature value: %s", f.c_str());
                std::string featureName;
                JSValue val;
                if (eq == std::string::npos) {
                    // default: set value to true
                    featureName = f;
                    val = JS_NewBool(jsCtx, true);
                } else {
                    // parse value as json
                    featureName = f.substr(0, eq);
                    std::string featureValue = f.substr(eq + 1);
                    val = JS_ParseJSON2(jsCtx, featureValue.c_str(),
                                        featureValue.size(), "<args>",
                                        JS_PARSE_JSON_EXT);
                    if (JS_IsException(val)) {
                        // using JS_GetException prevents throwing
                        JS_FreeValue(jsCtx, JS_GetException(jsCtx));
                        JS_FreeValue(jsCtx, val);
                        val = JS_NewStringLen(jsCtx, featureValue.c_str(),
                                              featureValue.size());
                    }
                }

                JSValue target = JS_DupValue(jsCtx, script.features);
                {
                    // handle dots and create nested objects (if the dot happens
                    // after an equals, ignore it)
                    size_t eqIdx = featureName.find("=");
                    size_t dotIdx = featureName.find(".");
                    while (dotIdx != std::string::npos &&
                           !(eqIdx != std::string::npos && eqIdx < dotIdx)) {
                        std::string parentName = featureName.substr(0, dotIdx);
                        featureName = featureName.substr(dotIdx + 1);
                        JSValue newTarget = JS_GetPropertyStr(
                            jsCtx, target, parentName.c_str());
                        if (JS_IsObject(newTarget)) {
                            JS_FreeValue(jsCtx, target);
                            target = newTarget;
                        } else {
                            // new target object doesn't exist; create it
                            if (!JS_IsUndefined(newTarget)) {
                                LOG_WARN("when setting feature value %s, "
                                         "nested feature value %s wasn't "
                                         "an object; overwriting",
                                         f.c_str(), parentName.c_str());
                            }
                            newTarget = JS_NewObject(jsCtx);
                            JS_SetPropertyStr(jsCtx, target, parentName.c_str(),
                                              JS_DupValue(jsCtx, newTarget));
                            JS_FreeValue(jsCtx, target);
                            target = newTarget;
                        }
                        dotIdx = featureName.find(".");
                    }
                }

                JS_SetPropertyStr(jsCtx, target, featureName.c_str(), val);
                JS_FreeValue(jsCtx, target);
            }
        }

        engine.cameras.resize(options.cameras.size());
        gameBootstrap(engine);

        for (auto &filename : options.jsFiles) {
            std::shared_ptr<std::string> contents = engine.getText(filename);
            engine.script.eval(filename.c_str(), contents->data(),
                               contents->size());
        }

        for (size_t i = 0; i < options.cameras.size(); ++i) {
            auto &camera = engine.cameras[i];
            auto &renderFn = options.cameras[i];
            if (renderFn) {
                JSValue jsRender =
                    JS_GetPropertyStr(script.ctx, script.exports, renderFn);
                if (JS_IsFunction(script.ctx, jsRender)) {
                    camera.render = [=]() {
                        _engine->script.callVoid(jsRender);
                    };
                    JS_FreeValue(script.ctx, jsRender);
                }
            }
        }

        {
            {
                JSValue jsUpdate =
                    JS_GetPropertyStr(jsCtx, script.exports, "update");
                if (JS_IsFunction(jsCtx, jsUpdate)) {
                    engine.onUpdate = [=]() {
                        _engine->script.callVoid(jsUpdate);
                    };
                    JS_FreeValue(jsCtx, jsUpdate);
                }
            }
            {
                JSValue jsFixedUpdate =
                    JS_GetPropertyStr(jsCtx, script.exports, "fixedUpdate");
                if (JS_IsFunction(jsCtx, jsFixedUpdate)) {
                    engine.onFixedUpdate = [=]() {
                        _engine->script.callVoid(jsFixedUpdate);
                    };
                    JS_FreeValue(jsCtx, jsFixedUpdate);
                }
            }
        }
    };

    _engine->run();

#ifndef __EMSCRIPTEN__
    // on most platforms, we're done here; in Emscripten, the main loop will
    // start running now
    _engine = nullptr;
#endif
}

}

int main(int argc, char *argv[]) {
    krit::gameMain(argc, argv);
    return 0;
}
