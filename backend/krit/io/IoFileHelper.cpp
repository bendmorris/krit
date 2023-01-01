#include "IoFileHelper.h"
#include "krit/utils/Panic.h"
#include <fstream>
#include <sstream>

namespace krit {

char *IoFile::read(const char *path, int *length) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.good()) {
        panic("file does not exist: %s\n", path);
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    char *buffer = (char *)alloc(size + 1);
    file.read(buffer, size);
    buffer[size] = 0;
    if (length) {
        *length = size;
    }
    return buffer;
}

void IoFile::write(const char *path, const char *buf, size_t size) {
    std::ofstream file(path, std::ios::binary | std::ios::out);
    file.write(buf, size);
}

bool IoFile::exists(const char *path) {
    std::ifstream infile(path);
    return infile.good();
}

void *IoFile::alloc(size_t size) { return std::malloc(size); }
void *IoFile::calloc(size_t size) { return std::calloc(size, 1); }
void *IoFile::realloc(void *p, size_t size) { return std::realloc(p, size); }
void IoFile::free(void *p) { std::free(p); }

}
