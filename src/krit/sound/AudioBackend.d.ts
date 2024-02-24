declare class AudioBackend {
    soundVolume: number;

    playSound(id: string): void;
    playSoundAsset(data: SoundData): void;
    playMusic(id: string): AudioStream;
}

declare class AudioStream {
    sampleRate(): number;
    currentPlayTime(): number;
    // seek(time: number): void;

    getVolume(): number;
    setVolume(v: number): number;

    play(): void;
    pause(): void;
    stop(): void;
    reset(): void;

    onLoop(id: string): void;
}
