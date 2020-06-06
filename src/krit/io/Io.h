#ifndef KRIT_IO_IO
#define KRIT_IO_IO

#include "krit/io/FileIo.h"
#include "krit/io/ZipIo.h"

namespace krit {

using IoRead = ZipIo;
using IoWrite = FileIo;

}

#endif