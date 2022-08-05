declare class DrawCommandBuffer {
    currentRenderTarget: FrameBuffer;
    setRenderTarget(fb: FrameBuffer, clear?: boolean): void;
    drawSceneShader(shader: SceneShader): void;
    clearColor(c: Color): void;
    pushClip(rect: Partial<Rectangle>): void;
    popClip(): void;
}
