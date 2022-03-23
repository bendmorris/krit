#include "krit/math/Random.h"

namespace krit {

std::random_device rd;
std::mt19937 rng(rd());

}
