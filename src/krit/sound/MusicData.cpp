#include "krit/sound/MusicData.h"
#include "krit/asset/AssetInfo.h"
#include "krit/asset/AssetLoader.h"
#include "krit/io/Io.h"
#include "krit/utils/Log.h"
#include <cstdint>
#include <memory.h>
#include <sndfile.h>

namespace krit {

MusicData::~MusicData() {
    if (sndFile) {
        sf_close(sndFile);
    }
    if (io.data) {
        ZipIo::free(io.data);
    }
}

}
