#ifndef KRIT_UTILS_PARSE
#define KRIT_UTILS_PARSE

#include "krit/math/Measurement.h"
#include "krit/utils/Color.h"
#include <string>

namespace krit {

struct ParseUtil {
    static Measurement parseMeasurement(std::string s) {
        bool percent = false;
        if (s.back() == '%') {
            percent = true;
            s.pop_back();
        }
        int value = std::stoi(s);
        return Measurement(percent ? Percent : Absolute, value);
    }
    static int parseInt(std::string &s, int base = 10) {
        return std::stoi(s, 0, base);
    }
    static double parseFloat(std::string s) {
        return std::stod(s, 0);
    }
    static Color parseColor(std::string s) {
        return Color(ParseUtil::parseInt(s, 16), 1.0);
    }
    static bool parseBool(std::string s) {
        return s == "true";
    }
};

}

#endif
