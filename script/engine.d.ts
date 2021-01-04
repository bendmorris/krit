/**
 * @namespace krit
 * @import krit/Engine.h
 * @pointerOnly
 */
interface Engine {
    input: InputContext;
}

/**
 * @namespace krit
 * @import krit/input/InputContext.h
 */
interface InputContext {
    /** @readonly */ events: ActionEvent[];
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
 * @import krit/UpdateContext.h
 */
interface UpdateContext {
    /** @readonly */ elapsed: number;
    camera: Pointer<Camera>;
    window: Pointer<IntDimensions>;
    engine: Pointer<Engine>;
}

/**
 * @namespace krit
 * @import krit/render/RenderContext.h
 */
interface RenderContext extends UpdateContext {}

/**
 * @namespace krit
 * @import krit/Camera.h
 */
interface Camera {
    position: Point;
    offset: Point;
    anchor: Point;
    dimensions: Dimensions;

    center(): void;
    transformPoint(p: Reference<Point>): void;
    untransformPoint(p: Reference<Point>): void;
}
