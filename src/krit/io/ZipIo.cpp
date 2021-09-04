#include "krit/io/ZipIo.h"

#include <stdlib.h>

#include "krit/io/FileIo.h"
#include "krit/utils/Panic.h"

namespace krit {

zip_t *ZipIo::archive = nullptr;

void ZipIo::setArchive(const std::string &path) {
    zip_source_t *source = zip_source_file_create(path.c_str(), 0, 0, nullptr);
    archive = zip_open_from_source(source, ZIP_RDONLY, nullptr);
}

bool ZipIo::exists(const std::string &path) {
    if (!archive) {
        return FileIo::exists(path);
    }
    int index = zip_name_locate(archive, path.c_str(), 0);
    return index != -1;
}

char *ZipIo::read(const std::string &path, int *length) {
    if (!archive) {
        return FileIo::read(path, length);
    }
    zip_stat_t stat;
    zip_stat_init(&stat);
    zip_stat(archive, path.c_str(), 0, &stat);
    int index = zip_name_locate(archive, path.c_str(), 0);
    if (index == -1) {
        return nullptr;
    }
    char *buffer = static_cast<char *>(malloc(stat.size + 1));
    zip_file_t *f = zip_fopen_index(archive, index, 0);
    if (!f) {
        panic("error opening file from archive: %s\n", path.c_str());
    }
    int bytes = stat.size;
    char *cur = buffer;
    while (bytes) {
        int read = zip_fread(f, cur, bytes);
        if (read == -1) {
            panic("error reading file from archive: %s\n", path.c_str());
        }
        bytes -= read;
        cur += read;
    }
    buffer[stat.size] = 0;
    if (length) {
        *length = stat.size;
    }
    zip_fclose(f);
    return buffer;
}

void ZipIo::free(char *buf) { free(buf); }

}