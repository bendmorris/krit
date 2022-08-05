declare class RenderContext extends UpdateContext {
    drawCommandBuffer: DrawCommandBuffer;

    width(): number;
    height(): number;
    pushClip(rect: Rectangle): void;
    popClip(): void;
    startAutoClip(): void;
    endAutoClip(): void;
    drawRect(x: number, y: number, w: number, h: number, c: number, a: number): void;
}
