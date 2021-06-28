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

struct StringSlice : public Slice<char> {
    StringSlice() : Slice() {}
    StringSlice(char *data, unsigned int length) : Slice(data, length) {}

    template <typename Arg> bool operator==(Arg other) {
        return this->cmp(other);
    }

    int cmp(char *other) { return strncmp(this->data, other, this->length); }

    int cmp(const std::string &other) { return this->cmp(other.c_str()); }

    int cmp(StringSlice &other) {
        return this->length == other.length &&
               strncmp(this->data, other.data, this->length);
    }
};

}

#endif
