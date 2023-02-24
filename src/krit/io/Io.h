#ifndef KRIT_IO
#define KRIT_IO

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>

namespace krit {

struct Io {
    virtual ~Io() = default;
    virtual void *alloc(size_t size) = 0;
    virtual void *calloc(size_t size) = 0;
    virtual void *realloc(void *p, size_t size) = 0;

    virtual char *read(const std::filesystem::path &path, int *length = nullptr) = 0;
    virtual void write(const std::filesystem::path &path, const char *buf,
                       size_t size) = 0;
    virtual bool exists(const std::filesystem::path &path) = 0;
    virtual bool isDirectory(const std::filesystem::path &path) = 0;
    virtual bool rm(const std::filesystem::path &path) = 0;
    virtual void free(void *p) = 0;
    virtual bool mkdir(const std::filesystem::path &path) = 0;

    std::string readFile(const std::filesystem::path &path) { return read(path.c_str()); }

    void writeFile(const std::filesystem::path &path, const std::string &contents) {
        write(path.c_str(), contents.c_str(), contents.size());
    }
};

std::unique_ptr<Io> io();

}

#endif
