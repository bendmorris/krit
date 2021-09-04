/**
 * @namespace krit
 * @import krit/render/RenderContext.h
 */
 interface RenderContext extends UpdateContext {
    drawCommandBuffer: Pointer<DrawCommandBuffer>;

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
declare class BaseFrameBuffer {}
 
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
    setRenderTarget(fb: Pointer<BaseFrameBuffer>): void;
    drawSceneShader(shader: Pointer<SceneShader>): void;
}
