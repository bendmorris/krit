#include "Renderer.h"

namespace krit {

struct KritRendererGl : public KritRenderer {
    KritRendererGl() {}
};

std::unique_ptr<KritRenderer> renderer() {
    return std::unique_ptr<KritRenderer>(new KritRendererGl());
}

}
