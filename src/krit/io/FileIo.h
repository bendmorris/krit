#ifndef KRIT_IO_FILEIO
#define KRIT_IO_FILEIO

#include "krit/utils/Panic.h"
#include <fstream>
#include <sstream>
#include <vector>

namespace krit {

struct FileIo {
    static std::string configDir();
    static std::string dataDir();
    static bool mkdir(const std::string &path);

    static char *read(const std::string &path, int *length = nullptr);
    static void write(const std::string &path, const char *buf, size_t size);
    static void write(const std::string &path, const std::string &buf) { write(path, buf.c_str(), buf.size()); }

    static void *alloc(size_t size) { return new char[size]; }
    static void free(char *buf) { delete[] buf; }

    static bool exists(const std::string &path) {
        std::ifstream infile(path);
        return infile.good();
    }
};

}

#endif
