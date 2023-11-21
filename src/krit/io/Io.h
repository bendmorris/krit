#ifndef KRIT_IO
#define KRIT_IO

#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace krit {

struct Io {
    virtual ~Io() = default;

    virtual bool addRoot(const std::filesystem::path &path) = 0;
    virtual bool hasRoot(const std::filesystem::path &path) = 0;
    virtual bool enableRoot(const std::filesystem::path &path) = 0;
    virtual bool disableRoot(const std::filesystem::path &path) = 0;

    virtual std::optional<std::filesystem::path> find(const std::filesystem::path &path) = 0;
    virtual bool exists(const std::filesystem::path &path) = 0;
    virtual std::string readFile(const std::filesystem::path &path) = 0;
    virtual std::vector<std::string> readDir(const std::filesystem::path &path) = 0;
};

std::unique_ptr<Io> io();

}

#endif
