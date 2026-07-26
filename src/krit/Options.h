#ifndef KRIT_OPTIONS
#define KRIT_OPTIONS

#include "krit/utils/Log.h"
#include <SDL3/SDL.h>
#include <string>
#include <vector>

namespace krit {

/**
 * Project option builder.
 */
struct KritOptions {
    std::string programName;
    std::string title;
    int width{320};
    int height{240};
    int fullscreenWidth{-1};
    int fullscreenHeight{-1};
    bool fullscreen{false};
    bool block{true};
    bool windowBorder{true};
    int fixedFramerate{60};
    void *userData{nullptr};
    SDL_PropertiesID windowProperties{0};
    std::vector<const char *> cameras;
#if KRIT_ENABLE_SCRIPT
    std::vector<std::string> jsFiles;
#endif
    LogLevel logLevel{LogLevel::Error};
    std::vector<std::string> features;
    std::vector<std::string> logAreas;

    KritOptions() {}

    KritOptions &setSize(int x, int y) {
        this->width = x;
        this->height = y;
        return *this;
    }
    KritOptions &setFullScreenSize(int x, int y) {
        this->fullscreenWidth = x;
        this->fullscreenHeight = y;
        return *this;
    }
    KritOptions &setProgramName(const std::string &p) {
        this->programName = p;
        return *this;
    }
    KritOptions &setTitle(const std::string &t) {
        this->title = t;
        return *this;
    }
    KritOptions &setFrameRate(int rate) {
        this->fixedFramerate = rate;
        return *this;
    }
    KritOptions &setFullscreen(bool val) {
        this->fullscreen = val;
        return *this;
    }
    KritOptions &addCamera(const char *renderExport) {
        cameras.push_back(renderExport);
        return *this;
    }
#if KRIT_ENABLE_SCRIPT
    KritOptions &addJs(std::string &&filename) {
        jsFiles.push_back(filename);
        return *this;
    }
#endif
};

}

#endif
