/**
 * @namespace krit
 * @import krit/UpdateContext.h
 */
interface UpdateContext {
    /** @readonly true */ elapsed: number,
    camera: Pointer<Camera>,
    window: Pointer<IntDimensions>,
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
    position: Point,
    offset: Point,
    anchor: Point,
}
