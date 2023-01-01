#ifndef KRIT_BACKEND_RENDERER
#define KRIT_BACKEND_RENDERER

#include <memory>

namespace krit {

struct KritRenderer {};

std::unique_ptr<KritRenderer> renderer();

}

#endif
