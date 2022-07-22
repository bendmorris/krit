/**
 * @namespace krit
 * @import krit/Engine.h
 * @pointerOnly
 */
interface Engine {
    input: InputContext;
    speed: number;
    elapsed: number;
    bgColor: Color;

    cameras: Array<Camera>;
    setCursor(c: string): void;
}

/**
 * @namespace krit
 * @import krit/Window.h
 * @pointerOnly
 */
interface Window extends IntDimensions {
    isFullScreen(): boolean;
    setFullScreen(full: boolean): void;
    setWindowSize(width: integer, height: integer): void;
}

/**
 * @namespace krit
 * @import krit/UpdateContext.h
 */
declare class UpdateContext {
    /** @readonly */ elapsed: number;
    /** @readonly */ frameId: number;
    camera: Pointer<Camera>;
    window: Pointer<Window>;
    engine: Pointer<Engine>;
    audio: Pointer<AudioBackend>;
}

/**
 * @namespace krit
 * @import krit/Camera.h
 */
declare class Camera {
    position: Point;
    offset: IntPoint;
    anchor: Point;
    scale: ScaleFactor;
    dimensions: Vec2f;
    currentDimensions: Vec2f;
    rotation: float;
    pitch: float;
    minRatio: number;
    maxRatio: number;

    center(): void;
    update(ctx: Reference<RenderContext>): void;
    transformPoint(p: Reference<Point>): void;
    untransformPoint(p: Reference<Point>): void;
    setLogicalSize(w: integer, h: integer): Reference<Camera>;
    viewportWidth(): integer;
    viewportHeight(): integer;

    screenToWorldCoords(p: Reference<Point>): void;
}
