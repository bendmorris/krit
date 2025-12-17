declare class Engine {
    readonly io: Io;
    readonly net: Net;
    readonly platform: Platform;
    readonly input: InputContext;
    readonly window: Window;
    readonly audio: AudioBackend;
    readonly fonts: FontManager;
    get scriptContext(): any;
    get isRenderPhase(): boolean;
    speed: number;
    // elapsed: number;
    totalElapsed: number;
    bgColor: Color;
    useSystemFullScreen: boolean;

    cameras: Array<Camera>;
    addCursor(path: string, name: string, resolution?: number, x?: number, y?: number): void;
    setCursor(name: string): void;

    getImage(id: string): ImageData;
    getAtlas(id: string): TextureAtlas;
    getAudio(id: string): AudioData;
}
