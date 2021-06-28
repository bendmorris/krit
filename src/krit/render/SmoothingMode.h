#ifndef KRIT_RENDER_SMOOTHING_MODE
#define KRIT_RENDER_SMOOTHING_MODE

namespace krit {

enum SmoothingMode: int {
    SmoothNearest,
    SmoothLinear,
    SmoothMipmap
};

}

#endif
