#include "krit/utils/Signal.h"

namespace krit {

void invoke(Signal s) {
    if (s) s();
}

}
