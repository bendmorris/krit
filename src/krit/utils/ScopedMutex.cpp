#include "ScopedMutex.h"

namespace krit {

ScopedMutex::ScopedMutex(std::mutex *m) : mutex(m) { mutex->lock(); }

ScopedMutex::~ScopedMutex() { mutex->unlock(); }

}