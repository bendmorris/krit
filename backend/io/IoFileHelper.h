#ifndef KRIT_IO_IOFILEHELPER
#define KRIT_IO_IOFILEHELPER

#include "krit/io/Io.h"

namespace krit {

struct IoFile : public Io {
    bool mkdir(const std::filesystem::path &path) override;
    char *read(const std::filesystem::path &path, int *length = nullptr) override;
    void write(const std::filesystem::path &path, const char *buf, size_t size) override;
    bool exists(const std::filesystem::path &path) override;
    bool isDirectory(const std::filesystem::path &path) override;
    bool rm(const std::filesystem::path &path) override;
    void *alloc(size_t size) override;
    void *calloc(size_t size) override;
    void *realloc(void *p, size_t size) override;
    void free(void *p) override;
};

}

#endif
