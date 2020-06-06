#ifndef KRIT_IO_ZIPIO
#define KRIT_IO_ZIPIO

#include "krit/utils/Panic.h"
#include "krit/io/FileIo.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <zip.h>

namespace krit {

struct ZipIo {
    static zip_t *archive;

    static void setArchive(const std::string &path) {
        int err = 0;
        archive = zip_open(path.c_str(), ZIP_RDONLY, &err);
        if (err) {
            panic("zip_open error setting archive %s: %i\n", path.c_str(), err);
        }
    }

    static char *read(const std::string &path, int *length = nullptr);

    static bool exists(const std::string &path) {
        int index = zip_name_locate(archive, path.c_str(), 0);
        return index != -1;
    }
};

}

#endif
