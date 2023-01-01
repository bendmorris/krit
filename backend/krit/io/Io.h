#ifndef KRIT_IO
#define KRIT_IO

#include <cstddef>
#include <memory>

namespace krit {

struct Io {
    virtual ~Io() = default;
    virtual char *read(const char *path, int *length = nullptr) = 0;
    virtual void write(const char *path, const char *buf, size_t size) = 0;
    virtual bool exists(const char *path) = 0;
    virtual void *alloc(size_t size) = 0;
    virtual void *calloc(size_t size) = 0;
    virtual void *realloc(void *p, size_t size) = 0;
    virtual void free(void *p) = 0;
};

std::unique_ptr<Io> io();

}

#endif
