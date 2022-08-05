declare class UpdateContext {
    readonly elapsed: number;
    readonly frameId: number;
    camera: Camera;
    window: Window;
    engine: Engine;
    audio: AudioBackend;
}
