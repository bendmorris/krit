#include "krit/io/FileIo.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace krit {

// int mkpath(char *dir, mode_t mode) {
//     struct stat sb;

//     if (!dir) {
//         errno = EINVAL;
//         return 1;
//     }

//     if (!stat(dir, &sb))
//         return 0;

//     mkpath(dirname(strdupa(dir)), mode);

//     return mkdir(dir, mode);
// }

std::string FileIo::configDir() {
    // FIXME: a platform abstraction that exposes the user data directory would
    // be nice here
    std::string path;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    // windows
    path = std::string(getenv("PROGRAMDATA"));
#elif __APPLE__
    // mac
    // TODO
#else
    // linux
    char *xdgConfig = getenv("XDG_CONFIG_HOME");
    if (xdgConfig) {
        path = std::string(xdgConfig);
    } else {
        path = std::string(getenv("HOME")) + "/.config";
    }
#endif
    // TODO: mkdir
    return path;
}

std::string FileIo::dataDir() {
    // FIXME: a platform abstraction that exposes the user data directory would
    // be nice here
    std::string path;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    // windows
    path = std::string(getenv("APPDATA"));
#elif __APPLE__
    // mac
    // TODO
#else
    // linux
    char *xdgConfig = getenv("XDG_DATA_HOME");
    if (xdgConfig) {
        path = std::string(xdgConfig);
    } else {
        path = std::string(getenv("HOME")) + "/.local/share";
    }
#endif
    // TODO: mkdir
    return path;
}

bool FileIo::mkdir(const std::string &path) {
    struct stat st = {0};
    if (stat("/some/directory", &st) == -1) {
        return ::mkdir(path.c_str(), 0700) == 0;
    } else {
        return true;
    }
}

char *FileIo::read(const std::string &path, int *length) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.good()) {
        panic("file does not exist: %s\n", path.c_str());
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    char *buffer = (char*)FileIo::alloc(size + 1);
    file.read(buffer, size);
    buffer[size] = 0;
    if (length) {
        *length = size;
    }
    return buffer;
}

void FileIo::write(const std::string &path, const char *buf, size_t size) {
    std::ofstream file(path, std::ios::binary | std::ios::out);
    file.write(buf, size);
}

}