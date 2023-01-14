#ifndef KRIT_IO_IOFILEHELPER
#define KRIT_IO_IOFILEHELPER

#include "krit/io/Io.h"

namespace krit {

struct IoFile : public Io {
    char *read(const char *path, int *length = nullptr) override;
    void write(const char *path, const char *buf, size_t size) override;
    bool exists(const char *path) override;
    bool rm(const char *path) override;
    void *alloc(size_t size) override;
    void *calloc(size_t size) override;
    void *realloc(void *p, size_t size) override;
    void free(void *p) override;
};

}

#endif
