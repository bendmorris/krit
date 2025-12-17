declare class AudioBackend {
    gain: number;

    oneShot(a: AudioData): StreamAudioSource;
    loop(a: AudioData): StreamAudioSource;
    sequence(): SequenceAudioSource;
    layered(): LayeredAudioSource;
}
