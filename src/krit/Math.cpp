#include "krit/Math.h"

namespace krit {

template <> float smoothStep<0>(float n) {
    return pow(n, 2) * (3 - 2 * n);
}

}
