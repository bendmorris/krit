/**
 * @namespace krit
 * @import krit/render/RenderContext.h
 */
declare class RenderContext extends UpdateContext {
    drawCommandBuffer: Pointer<DrawCommandBuffer>;

    width(): integer;
    height(): integer;
    pushClip(rect: Reference<Rectangle>): void;
    popClip(): void;
    startAutoClip(): void;
    endAutoClip(): void;
    drawRect(x: integer, y: integer, w: integer, h: integer, c: number, a: float): void;
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
    allowSmoothing: boolean;

    constructor(width: integer, height: integer, multisample: boolean);

    resize(width: integer, height: integer): void;
    readPixel(x: integer, y: integer): integer;
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
    pushClip(rect: Reference<Rectangle>): void;
    popClip(): void;
}

/**
 * @namespace krit
 * @import krit/render/ImageRegion.h
 */
declare class ImageRegion {
    rect: IntRectangle;
}

/**
 * @namespace krit
 * @import krit/render/DrawContext.h
 */
declare class DrawContext {
    /** @cast BlendMode */ blend: BlendMode;
    /** @cast SmoothingMode */ smooth: SmoothingMode;
    color: Color;
    alpha: float;
    lineThickness: float;
    zIndex: integer;

    constructor(ctx: Reference<RenderContext>);

    line(p1: Vec2f, p2: Vec2f): void;
    // polyline()
    rect(r: Reference<Rectangle>): void;
    rectFilled(r: Reference<Rectangle>): void;
    circle(center: Reference<Vec2f>, radius: float, segments: size_t): void;
    circleFilled(center: Reference<Vec2f>, radius: float, segments: size_t): void;
    arc(center: Reference<Vec2f>, radius: float, startRads: float, angleRads: float, segments: size_t): void;
}
