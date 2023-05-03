declare class Engine {
    get renderCtx(): RenderContext;
    get updateCtx(): UpdateContext;

    readonly io: Io;
    readonly net: Net;
    readonly platform: Platform;
    readonly input: InputContext;
    readonly window: Window;
    readonly audio: AudioBackend;
    readonly fonts: FontManager;
    get scriptContext(): any;
    speed: number;
    // elapsed: number;
    totalElapsed: number;
    bgColor: Color;

    cameras: Array<Camera>;
    addCursor(path: string, name: string, resolution: number): void;
    setCursor(name: string): void;

    getImage(id: string): ImageData;
    getAtlas(id: string): TextureAtlas;
}
