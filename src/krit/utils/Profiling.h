#pragma once

#if TRACY_ENABLE
#include "Tracy.hpp"
#define ProfileZone(n) ZoneScopedN(n)
#else
#define ProfileZone(n)                                                         \
    do {                                                                       \
    } while (false);
#endif
