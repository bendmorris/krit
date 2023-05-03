#include "Platform.h"
#include "krit/utils/Panic.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

namespace krit {

bool Platform::exists(const std::filesystem::path &path) {
    return std::filesystem::exists(path);
}

bool Platform::isDir(const std::filesystem::path &path) {
    return std::filesystem::is_directory(path);
}

std::string Platform::readFile(const std::filesystem::path &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.good()) {
        // TODO: handle errors
        auto s = path.string();
        panic("file does not exist: %s\n", s.c_str());
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string buffer;
    buffer.resize(size);
    file.read(buffer.data(), size);
    return buffer;
}

void Platform::writeFile(const std::filesystem::path &path,
                         const std::string &content) {
    std::ofstream file(path.string().c_str(), std::ios::binary | std::ios::out);
    file.write(content.c_str(), content.size());
}

bool Platform::copyFile(const std::filesystem::path &src,
                        const std::filesystem::path &dest) {
    return std::filesystem::copy_file(src, dest);
}

bool Platform::moveFile(const std::filesystem::path &src,
                        const std::filesystem::path &dest) {
    std::filesystem::rename(src, dest);
    return true;
}

bool Platform::createDir(const std::filesystem::path &path, bool recursive) {
    if (recursive) {
        return std::filesystem::create_directories(path);
    } else {
        return std::filesystem::create_directory(path);
    }
}

std::vector<std::string>
Platform::readDir(const std::filesystem::path &path) {
    std::vector<std::string> results;
    for (auto &entry : std::filesystem::directory_iterator(path)) {
        results.push_back(entry.path().string());
    }
    return results;
}

bool Platform::remove(const std::filesystem::path &path, bool recursive) {
    if (recursive) {
        return std::filesystem::remove_all(path) > 0;
    } else {
        return std::filesystem::remove(path);
    }
}

}
