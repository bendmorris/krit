#ifndef KRIT_IO_ZIPIO
#define KRIT_IO_ZIPIO

#include <zip.h>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "krit/utils/Panic.h"
#include "krit/io/FileIo.h"

namespace krit {

struct ZipIo {
    static void setArchive(const std::string &path);

    static char *read(const std::string &path, int *length = nullptr);
    static void free(char *buf);
    static bool exists(const std::string &path);

    private:
        static zip_t *archive;
};

}

#endif
