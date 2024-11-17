#ifndef KRIT_UPDATE_CONTEXT
#define KRIT_UPDATE_CONTEXT

namespace krit {

struct UpdateContext {
    float elapsed { 0 };
    unsigned int frameCount { 0 };
    unsigned int frameId { 0 };
    unsigned int tickId { 0 };

    UpdateContext() {}
};

extern UpdateContext frame;

}

#endif
