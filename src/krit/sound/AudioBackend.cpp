#include "krit/sound/AudioBackend.h"
#include "krit/App.h"
#include "krit/sound/SoundData.h"
#include "krit/utils/Log.h"

namespace krit {

AudioBackend::AudioBackend() {
    this->device = alcOpenDevice(nullptr);
    if (!this->device) {
        panic("couldn't open OpenAL default device");
    }

    this->context = alcCreateContext(this->device, nullptr);
    if (!this->context) {
        panic("couldn't create OpenAL context");
    }
    if (!alcMakeContextCurrent(this->context)) {
        panic("couldn't set OpenAL context");
    }

    alGenBuffers(4, buffers);
}

AudioBackend::~AudioBackend() {
    alcCloseDevice(device);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(this->context);
    alcCloseDevice(this->device);
}

void AudioBackend::playSound(const std::string &name) {
    playSound(App::ctx.engine->getSound(name).get());
}

void AudioBackend::playSound(const AssetInfo &info) {
    playSound(App::ctx.engine->getSound(info).get());
}

void AudioBackend::playSound(SoundData *sound) {
    ALenum format;
    if (sound->channels == 1) {
        format = AL_FORMAT_MONO16;
    } else if (sound->channels == 2) {
        format = AL_FORMAT_STEREO16;
    } else {
        Log::error("unsupported number of channels: %i", sound->channels);
        return;
    }
    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, sound->data, sound->frames * sound->channels * 2, sound->sampleRate);
    ALuint source;
    alGenSources(1, &source);
    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, 1.0f);
    alSource3f(source, AL_POSITION, 0, 0, 0);
    alSource3f(source, AL_VELOCITY, 0, 0, 0);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, buffer);
    alSourcePlay(source);
}

}