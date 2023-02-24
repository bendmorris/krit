#ifndef KRIT_OPTIONS
#define KRIT_OPTIONS

#include <string>

namespace krit {

/**
 * Project option builder.
 */
struct KritOptions {
    std::string title;
    int width = 320;
    int height = 240;
    int fullscreenWidth = -1;
    int fullscreenHeight = -1;
    bool fullscreen = false;
    int fixedFramerate;
    void *userData;

    KritOptions() {}

    KritOptions &setSize(int x, int y) {
        this->width = x;
        this->height = y;
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
};

}

#endif
