#pragma once
#include <mutex>

namespace krit {

struct ScopedMutex {
    ScopedMutex(std::mutex *);
    ~ScopedMutex();

    private:
    std::mutex *mutex;
};

}