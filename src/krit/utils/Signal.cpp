#include "krit/utils/Signal.h"

using namespace krit;

void invoke(Signal s) {
    if (s) s();
}
