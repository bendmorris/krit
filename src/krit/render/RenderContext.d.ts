declare class RenderContext extends UpdateContext {
    drawCommandBuffer: DrawCommandBuffer;
    camera: Camera;

    width(): number;
    height(): number;
    pushClip(rect: Rectangle): void;
    popClip(): void;
    startAutoClip(): void;
    endAutoClip(): void;
    drawRect(x: number, y: number, w: number, h: number, c: Color, a: number): void;
}
