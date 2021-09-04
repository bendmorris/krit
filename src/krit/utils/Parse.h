#ifndef KRIT_UTILS_PARSE
#define KRIT_UTILS_PARSE

#include "krit/math/Measurement.h"
#include "krit/utils/Color.h"
#include "krit/utils/Panic.h"
#include <cassert>
#include <string>
#include <unordered_map>

namespace krit {

struct Parse {
    static void defineConstant(const std::string &name,
                               const std::string &value) {
        assert(name.c_str()[0] == '$');
        constants.insert(std::make_pair(name, value));
    }

    template <typename T> static T parse(const std::string &s);
    static int parseInt(const std::string &s, int base = 10);

private:
    static std::unordered_map<std::string, std::string> constants;

    static const std::string &_preParse(const std::string &val) {
        if (val[0] == '$') {
            auto found = constants.find(val);
            if (found == constants.end()) {
                panic("missing constant: %s", val.c_str());
            }
            return found->second;
        }
        return val;
    }
};

template <> const std::string &Parse::parse(const std::string &_s);
template <> Measurement Parse::parse(const std::string &_s);
template <> int Parse::parse(const std::string &_s);
template <> float Parse::parse(const std::string &_s);
template <> Color Parse::parse(const std::string &_s);
template <> bool Parse::parse(const std::string &_s);

}

#endif
