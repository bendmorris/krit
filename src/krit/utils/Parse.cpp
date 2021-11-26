#include "krit/utils/Parse.h"
#include <string>
#include <unordered_map>

namespace krit {

std::unordered_map<std::string, std::string> Parse::constants;

int Parse::parseInt(const std::string &_s, int base) {
    const std::string &s = _preParse(_s);
    return std::stoi(s, 0, base);
}

template <> bool Parse::parse(const std::string &_s) {
    const std::string &s = _preParse(_s);
    assert(s == "true" || s == "false");
    return s == "true";
}

template <> int Parse::parse(const std::string &_s) {
    return Parse::parseInt(_s, 10);
}

template <> float Parse::parse(const std::string &_s) {
    const std::string &s = _preParse(_s);
    return std::stof(s, 0);
}

template <> const std::string &Parse::parse(const std::string &_s) {
    return _preParse(_s);
}

template <> Measurement Parse::parse(const std::string &_s) {
    std::string s = _preParse(_s);
    bool percent = false;
    if (s.back() == '%') {
        percent = true;
        s.pop_back();
    }
    int value = std::stoi(s);
    return Measurement(percent ? Percent : Absolute, value);
}

template <> Color Parse::parse(const std::string &_s) {
    const std::string &s = _preParse(_s);
    return Color(Parse::parseInt(s, 16), 1.0);
}

template <> SmoothingMode Parse::parse(const std::string &_s) {
    const std::string &s = _preParse(_s);
    if (s == "linear") {
        return SmoothLinear;
    } else if (s == "mipmap") {
        return SmoothMipmap;
    } else if (s == "nearest") {
        return SmoothNearest;
    } else {
        panic("unrecognized smoothing mode: %s", s.c_str());
    }
}

}
