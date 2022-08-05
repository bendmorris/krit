declare class FrameBuffer {
    multisample: boolean;
    allowSmoothing: boolean;

    constructor(width: number, height: number, multisample: boolean);

    resize(width: number, height: number): void;
    readPixel(x: number, y: number): number;
}
