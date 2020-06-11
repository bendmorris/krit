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
    static void setArchive(const std::string &path);

    static char *read(const std::string &path, int *length = nullptr);

    static bool exists(const std::string &path) {
        int index = zip_name_locate(archive, path.c_str(), 0);
        return index != -1;
    }

    private:
        static zip_t *archive;
};

}

#endif
