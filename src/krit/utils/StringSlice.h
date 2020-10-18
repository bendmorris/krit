#ifndef KRIT_UTILS_STRINGSLICE
#define KRIT_UTILS_STRINGSLICE

#include <cstring>
#include <string>

namespace krit {

struct StringSlice {
    char *str = nullptr;
    unsigned int length = 0;

    StringSlice() {}
    StringSlice(char *str, unsigned int length): str(str), length(length) {}

    void setTo(char *str, unsigned int length) {
        this->str = str;
        this->length = length;
    }

    template<typename T> bool operator==(T other) { return this->cmp(other); }

    int cmp(char *other) {
        return strncmp(this->str, other, this->length);
    }

    int cmp(const std::string &other) {
        return this->cmp(other.c_str());
    }

    int cmp(StringSlice &other) {
        return this->length == other.length && strncmp(this->str, other.str, this->length);
    }

    char &operator[](int index) { return this->str[index]; }
};

}

#endif
