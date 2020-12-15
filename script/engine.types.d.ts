/**
 * @namespace krit
 * @import krit/UpdateContext.h
 */
interface UpdateContext {
    /** @readonly true */ elapsed: number,
}

/**
 * @namespace krit
 * @import krit/render/RenderContext.h
 */
interface RenderContext extends UpdateContext {}
