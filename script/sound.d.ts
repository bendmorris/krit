/**
 * @namespace krit
 * @import krit/sound/AudioBackend.h
 * @pointerOnly
 */
interface AudioBackend {
    playSound(id: string): void;
    playMusic(id: string): Pointer<AudioStream>;
}

/**
 * @namespace krit
 * @import krit/sound/AudioBackend.h
 * @pointerOnly
 */
interface AudioStream {
    getVolume(): float;
    setVolume(v: float): float;

    play(): void;
    pause(): void;
    stop(): void;
    reset(): void;
}
