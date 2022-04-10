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
 * @import krit/input/InputContext.h
 */
interface InputContext {
    /** @readonly */ events: ActionEvent[];
    mouse: MouseContext;
    state(a: integer): integer;
}

/**
 * @namespace krit
 * @import krit/input/InputContext.h
 */
interface MouseContext {
    mousePos: Point;
}

/**
 * @namespace krit
 * @import krit/input/InputContext.h
 */
interface ActionEvent {
    action: integer;
    state: integer;
    prevState: integer;
}

/**
 * @namespace krit
 * @import krit/Window.h
 * @pointerOnly
 */
interface Window extends IntDimensions {}

/**
 * @namespace krit
 * @import krit/UpdateContext.h
 */
interface UpdateContext {
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
interface Camera {
    position: Point;
    offset: Point;
    anchor: Point;
    scale: ScaleFactor;
    dimensions: Vec2f;
    rotation: float;
    pitch: float;

    center(): void;
    update(ctx: Reference<RenderContext>): void;
    transformPoint(p: Reference<Point>): void;
    untransformPoint(p: Reference<Point>): void;
    setLogicalSize(w: integer, h: integer): Reference<Camera>;
    keepWidth(minHeight: integer, maxHeight: integer): Reference<Camera>;
    keepHeight(minWidth: integer, maxWidth: integer): Reference<Camera>;

    screenToWorldCoords(p: Reference<Point>): void;
}
