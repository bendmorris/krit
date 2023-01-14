declare class Engine {
    get renderCtx(): RenderContext;
    get updateCtx(): UpdateContext;

    readonly io: Io;
    readonly net: Net;
    readonly platform: Platform;
    readonly input: InputContext;
    readonly window: Window;
    readonly audio: AudioBackend;
    readonly scriptContext: any;
    speed: number;
    // elapsed: number;
    totalElapsed: number;
    bgColor: Color;

    cameras: Array<Camera>;
    setCursor(c: string): void;

    getImage(id: string): ImageData;
    getAtlas(id: string): TextureAtlas;
}
