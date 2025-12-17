#ifndef KRIT_OPTIONS
#define KRIT_OPTIONS

#include <string>
#include <vector>
#include "krit/utils/Log.h"

namespace krit {

/**
 * Project option builder.
 */
struct KritOptions {
    std::string programName;
    std::string title;
    int width { 320 };
    int height { 240 };
    int fullscreenWidth { -1 };
    int fullscreenHeight { -1 };
    bool fullscreen { false };
    int fixedFramerate { 60 };
    void *userData { nullptr };
    std::vector<const char*> cameras;
    std::vector<std::string> jsFiles;
    LogLevel logLevel { LogLevel::Error };
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
    KritOptions &addJs(std::string &&filename) {
        jsFiles.push_back(filename);
        return *this;
    }
};

}

#endif
