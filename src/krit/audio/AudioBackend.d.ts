declare class AudioBackend {
    gain: number;

    oneShot(a: SharedPtr<AudioData>): SharedPtr<StreamAudioSource>;
    loop(a: SharedPtr<AudioData>): SharedPtr<StreamAudioSource>;
    sequence(): SharedPtr<SequenceAudioSource>;
    layered(): SharedPtr<LayeredAudioSource>;
}
