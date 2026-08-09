declare class Engine {
    readonly io: Io;
    readonly net: Net;
    readonly platform: Platform;
    readonly input: InputContext;
    readonly window: Window;
    readonly audio: AudioBackend;
    readonly fonts: FontManager;
    readonly scriptContext: any;
    readonly isRenderPhase: boolean;
    speed: number;
    // elapsed: number;
    totalElapsed: number;
    bgColor: Color;

    cameras: Array<Camera>;
    addCursor(path: string, name: string, /** @defaultValue 0 */ resolution?: number, /** @defaultValue 0 */ x?: number, /** @defaultValue 0 */ y?: number): void;
    setCursor(name: string): void;

    getImage(id: string): SharedPtr<ImageData>;
    getAtlas(id: string): SharedPtr<TextureAtlas>;
    getAudio(id: string): SharedPtr<AudioData>;
}
