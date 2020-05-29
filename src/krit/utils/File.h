#ifndef KRIT_UTILS_FILE
#define KRIT_UTILS_FILE

#include "krit/utils/Panic.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace krit {

struct File {
    static char *read(const std::string &path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.good()) {
            panic("file does not exist: %s\n", path.c_str());
        }
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        char *buffer = new char[size + 1];
        file.read(buffer, size);
        buffer[size] = 0;
        return buffer;
    }

    static bool exists(const std::string &path) {
        std::ifstream infile(path);
        return infile.good();
    }
};

}

#endif
