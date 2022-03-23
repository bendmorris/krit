#include <cstdint>
#include <memory.h>
#include <sndfile.h>

#include "krit/asset/Assets.h"
#include "krit/asset/AssetLoader.h"
#include "krit/io/Io.h"
#include "krit/sound/SoundData.h"
#include "krit/utils/Log.h"

namespace krit {

SoundData::~SoundData() {
    // FIXME: to destroy a buffer, we need to end all sources playing it first,
    // so the AudioBackend should manage deletion of buffers
    if (buffer) {
        alDeleteBuffers(1, &buffer);
    }
}

}
