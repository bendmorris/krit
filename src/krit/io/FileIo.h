#ifndef KRIT_IO_FILEIO
#define KRIT_IO_FILEIO

#include "krit/utils/Panic.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace krit {

struct FileIo {
    static char *read(const std::string &path, int *length = nullptr) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.good()) {
            panic("file does not exist: %s\n", path.c_str());
        }
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        char *buffer = static_cast<char*>(malloc(size + 1));
        file.read(buffer, size);
        buffer[size] = 0;
        if (length) {
            *length = size;
        }
        return buffer;
    }

    static bool exists(const std::string &path) {
        std::ifstream infile(path);
        return infile.good();
    }
};

}

#endif
