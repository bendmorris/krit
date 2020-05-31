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
    int fps = 60;
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

    KritOptions &setFrameRate(int f) {
        this->fps = f;
        return *this;
    }
};

}

#endif
