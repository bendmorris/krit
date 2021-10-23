/**
 * @namespace krit
 * @import krit/render/RenderContext.h
 */
 interface RenderContext extends UpdateContext {
    drawCommandBuffer: Pointer<DrawCommandBuffer>;

    width(): integer;
    height(): integer;
    pushClip(rect: Rectangle): void;
    popClip(): void;
}

/**
 * @namespace krit
 * @import krit/render/SpriteShader.h
 */
declare class SpriteShader {}
 
/**
 * @namespace krit
 * @import krit/render/FrameBuffer.h
 */
declare class BaseFrameBuffer {
    resize(width: integer, height: integer): void;
}

/**
 * @namespace krit
 * @import krit/render/SceneShader.h
 */
declare class SceneShader {}

/**
 * @namespace krit
 * @import krit/render/DrawCommand.h
 */
declare class DrawCommandBuffer {
    currentRenderTarget: Pointer<BaseFrameBuffer>;
    setRenderTarget(fb: Pointer<BaseFrameBuffer>): void;
    drawSceneShader(shader: Pointer<SceneShader>): void;
    clearColor(r: float, g: float, b: float, a: float): void;
}
