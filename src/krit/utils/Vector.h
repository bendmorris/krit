#ifndef KRIT_UTILS_VECTOR
#define KRIT_UTILS_VECTOR

namespace krit {

template <typename T> struct Vector {
    static Vector<T> alloc(size_t length) {
        Vector<T> *vector =
            static_cast<Vector<T> *> malloc(offsetof(Vector<T>, data[length]));
        vector->length = length;
        return vector;
    }

    const size_t length;
    T data[1];
};

}

#endif
