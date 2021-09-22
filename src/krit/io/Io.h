#ifndef KRIT_IO_IO
#define KRIT_IO_IO

#include "krit/io/FileIo.h"
#include "krit/io/ZipIo.h"

namespace krit {

using IoRead = ZipIo;
// using IoRead = FileIo;
using IoWrite = FileIo;

}

#endif