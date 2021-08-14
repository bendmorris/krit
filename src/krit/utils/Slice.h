#ifndef KRIT_UTILS_SLICE
#define KRIT_UTILS_SLICE

#include <cstring>
#include <string>

namespace krit {

template <typename T> struct Slice {
    size_t length = 0;
    T *data = nullptr;

    Slice() {}
    Slice(T *data, unsigned int length) : length(length), data(data) {}

    void setTo(T *data, unsigned int length) {
        this->data = data;
        this->length = length;
    }

    T &operator[](int index) { return this->data[index]; }
};

}

#endif
