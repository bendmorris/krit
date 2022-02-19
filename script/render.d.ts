/**
 * @namespace krit
 * @import krit/render/RenderContext.h
 */
interface RenderContext extends UpdateContext {
    drawCommandBuffer: Pointer<DrawCommandBuffer>;

    width(): integer;
    height(): integer;
    pushClip(rect: Reference<Rectangle>): void;
    pushDynamicClip(rect: Reference<Rectangle>): void;
    popClip(): void;
    pushBounds(rect: Reference<Rectangle>): void;
    popBounds(): void;
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
declare class FrameBuffer {
    multisample: boolean;

    constructor(width: integer, height: integer, multisample: boolean);

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
    currentRenderTarget: Pointer<FrameBuffer>;
    setRenderTarget(fb: Pointer<FrameBuffer>, clear?: boolean): void;
    drawSceneShader(shader: Pointer<SceneShader>): void;
    clearColor(r: float, g: float, b: float, a: float): void;
}

/**
 * @namespace krit
 * @import krit/render/ImageRegion.h
 */
declare class ImageRegion {
    rect: IntRectangle;
}
