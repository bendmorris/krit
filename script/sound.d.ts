/**
 * @namespace krit
 * @import krit/sound/AudioBackend.h
 */
interface AudioBackend {
    playSound(id: string): void;
    playMusic(id: string): Pointer<AudioStream>;
}

/**
 * @namespace krit
 * @import krit/sound/AudioBackend.h
 */
interface AudioStream {
    play(): void;
    pause(): void;
    stop(): void;
}
