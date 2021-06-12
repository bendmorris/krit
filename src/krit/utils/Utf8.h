#include <string>

namespace krit {

/**
 * Adapted from http://www.nubaria.com/en/blog/?p=371
 */
struct Utf8Iterator
    : public std::iterator<std::bidirectional_iterator_tag, char32_t,
                           std::string::difference_type, const char32_t *,
                           const char32_t &> {
    const unsigned char kFirstBitMask   = 0b10000000;
    const unsigned char kSecondBitMask  = 0b01000000;
    const unsigned char kThirdBitMask   = 0b00100000;
    const unsigned char kFourthBitMask  = 0b00010000;
    const unsigned char kFifthBitMask   = 0b00001000;

    Utf8Iterator(std::string::iterator it)
        : begin(it), it(it) {}
    Utf8Iterator(std::string::const_iterator it)
        : begin(it), it(it) {}
    Utf8Iterator(const Utf8Iterator &source)
        : begin(source.begin), it(source.it) {}

    Utf8Iterator &operator=(const Utf8Iterator &rhs) {
        it = rhs.it;
        return *this;
    }

    Utf8Iterator &operator++() {
        char firstByte = *it;
        size_t offset = 1;

        if (firstByte & kFirstBitMask) {
            if (firstByte & kThirdBitMask) {
                offset = (firstByte & kFourthBitMask) ? 4 : 3;
            } else {
                offset = 2;
            }
        }

        it += offset;
        return *this;
    }

    Utf8Iterator operator++(int) {
        Utf8Iterator temp = *this;
        ++(*this);
        return temp;
    }

    Utf8Iterator &operator--() {
        --it;
        if (*it & kFirstBitMask) {
            --it;
            if ((*it & kSecondBitMask) == 0) {
                --it;
                if ((*it & kSecondBitMask) == 0) {
                    --it;
                }
            }
        }
        return *this;
    }

    Utf8Iterator operator--(int) {
        Utf8Iterator temp = *this;
        --(*this);
        return temp;
    }

    char32_t operator*() const {
        char32_t codePoint = 0;
        char firstByte = *it;

        if (firstByte & kFirstBitMask) {
            if (firstByte & kThirdBitMask) {
                if (firstByte & kFourthBitMask) {
                    codePoint = (firstByte & 0x07) << 18;
                    char secondByte = *(it + 1);
                    codePoint += (secondByte & 0x3f) << 12;
                    char thirdByte = *(it + 2);
                    codePoint += (thirdByte & 0x3f) << 6;
                    ;
                    char fourthByte = *(it + 3);
                    codePoint += (fourthByte & 0x3f);
                } else {
                    codePoint = (firstByte & 0x0f) << 12;
                    char secondByte = *(it + 1);
                    codePoint += (secondByte & 0x3f) << 6;
                    char thirdByte = *(it + 2);
                    codePoint += (thirdByte & 0x3f);
                }
            } else {
                codePoint = (firstByte & 0x1f) << 6;
                char secondByte = *(it + 1);
                codePoint += (secondByte & 0x3f);
            }
        } else {
            codePoint = firstByte;
        }

        return codePoint;
    }

    bool operator==(const Utf8Iterator &rhs) const {
        return it == rhs.it;
    }

    bool operator!=(const Utf8Iterator &rhs) const {
        return it != rhs.it;
    }

    bool operator==(std::string::iterator rhs) const {
        return it == rhs;
    }

    bool operator==(std::string::const_iterator rhs) const {
        return it == rhs;
    }

    bool operator!=(std::string::iterator rhs) const {
        return it != rhs;
    }

    bool operator!=(std::string::const_iterator rhs) const {
        return it != rhs;
    }

    size_t index() { return it - begin; }

private:
    std::string::const_iterator begin;
    std::string::const_iterator it;
};

}
