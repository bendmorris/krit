#include "krit/utils/Interpolation.h"

namespace krit {

std::unordered_map<std::string, float (*)(float)> interpolationFunctions{
    {"easeOutSine", easeOutSine<float>},
    {"easeInSine", easeInSine<float>},
    {"easeInQuad", easeInQuad<float>},
    {"easeOutQuad", easeOutQuad<float>},
    {"easeInOutQuad", easeInOutQuad<float>},
    {"easeInCubic", easeInCubic<float>},
    {"easeOutCubic", easeOutCubic<float>},
    {"easeInOutCubic", easeInOutCubic<float>},
    {"easeInQuart", easeInQuart<float>},
    {"easeOutQuart", easeOutQuart<float>},
    {"easeInOutQuart", easeInOutQuart<float>},
    {"easeInQuint", easeInQuint<float>},
    {"easeOutQuint", easeOutQuint<float>},
    {"easeInOutQuint", easeInOutQuint<float>},
    {"easeOutBounce", easeOutBounce<float>},
};

}
