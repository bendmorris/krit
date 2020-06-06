#include "krit/io/ZipIo.h"

namespace krit {

zip_t *ZipIo::archive = nullptr;

char *ZipIo::read(const std::string &path, int *length) {
    zip_stat_t stat;
    zip_stat_init(&stat);
    zip_stat(archive, path.c_str(), 0, &stat);
    int index = zip_name_locate(archive, path.c_str(), 0);
    if (index == -1) {
        return nullptr;
    }
    char *buffer = new char[stat.size + 1];
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

}