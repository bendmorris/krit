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
}

declare class StreamAudioSource extends AudioSource {
    readonly stream: AudioStream;
}

declare class SequenceAudioSource extends AudioSource {
    readonly parts: Array<AudioStream>;
    loopStart: number;

    addPart(data: AudioData): SequenceAudioSource;
}

declare class LayeredAudioSource extends AudioSource {
    readonly layers: Array<AudioSource>;
    addLayer(source: AudioSource): LayeredAudioSource;
}
