#ifndef KRIT_IO_IO
#define KRIT_IO_IO

#include "krit/io/FileIo.h"
#include "krit/io/ZipIo.h"

namespace krit {

#ifdef __EMSCRIPTEN__
using IoRead = FileIo;
#else
using IoRead = ZipIo;
#endif
using IoWrite = FileIo;

}

#endif