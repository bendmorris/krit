declare class DrawCommandBuffer {
    currentRenderTarget: FrameBuffer;
    setRenderTarget(fb: Ptr<FrameBuffer>, /** @defaultValue false */ clear?: boolean): void;
    drawSceneShader(shader: Ptr<SceneShader>): void;
    clearColor(c: Color): void;
    pushClip(rect: Partial<Rectangle>): void;
    popClip(): void;
    queueReadPixel(fb: Ptr<FrameBuffer>, x: int, y: int): void;
}
