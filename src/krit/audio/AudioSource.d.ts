declare class AudioStream {
    gain: number;
    position: Vec3f;
}

declare class AudioSource {
    gain: number;
    loop: boolean;
    loopFrom: number;
    position: Vec3f;

    play(): void;
    stop(): void;
    pause(): void;
    playing(): boolean;

    get lowpass(): float;
    set lowpass(v: float);
    get highpass(): float;
    set highpass(v: float);
}

declare class StreamAudioSource extends AudioSource {
    readonly stream: AudioStream;
}

declare class SequenceAudioSource extends AudioSource {
    readonly parts: Array<AudioStream>;
    loopStart: number;

    addPart(data: SharedPtr<AudioData>): SharedPtr<SequenceAudioSource>;
}

declare class LayeredAudioSource extends AudioSource {
    readonly layers: Array<AudioSource>;
    addLayer(source: SharedPtr<AudioSource>): SharedPtr<LayeredAudioSource>;
    select(layerIndex: size_t): void;
}
