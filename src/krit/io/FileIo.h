#ifndef KRIT_IO_FILEIO
#define KRIT_IO_FILEIO

#include "krit/utils/Panic.h"
#include <fstream>
#include <sstream>
#include <vector>

namespace krit {

struct FileIo {
    static std::string dataDir();

    static char *read(const std::string &path, int *length = nullptr);
    static void write(const std::string &path, char *buf, size_t size);

    static void *alloc(size_t size) { return malloc(size); }
    static void free(char *buf) { std::free(buf); }

    static bool exists(const std::string &path) {
        std::ifstream infile(path);
        return infile.good();
    }
};

}

#endif
