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
    int windowWidth = -1;
    int windowHeight = -1;
    int framerate;
    int fixedFramerate;
    void *userData;

    KritOptions() {}

    KritOptions &setSize(int x, int y) {
        this->width = x;
        this->height = y;
        return *this;
    }

    KritOptions &setWindowSize(int x, int y) {
        this->windowWidth = x;
        this->windowHeight = y;
        return *this;
    }

    KritOptions &setTitle(const std::string &t) {
        this->title = t;
        return *this;
    }

    KritOptions &setFrameRate(int free, int fixed) {
        this->framerate = free;
        this->fixedFramerate = fixed;
        return *this;
    }
};

}

#endif
