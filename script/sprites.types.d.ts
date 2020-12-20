/**
 * @namespace krit
 * @import krit/Sprite.h
 */
interface Sprite {
    update: (ctx: Reference<UpdateContext>) => void,
    render: (ctx: Reference<RenderContext>) => void,
}

/**
 * @namespace krit
 * @import krit/Sprite.h
 */
interface VisibleSprite extends Sprite {
    /** @readonly */ position: Point,
    /** @readonly */ dimensions: Dimensions,
    /** @readonly */ scale: ScaleFactor,
    color: Color,

    /** @convert */ toBackdrop: () => Backdrop,
    /** @convert */ toBitmapText: () => BitmapText,
    /** @convert */ toImage: () => Image,
    /** @convert */ toLayoutNode: () => LayoutNode,
    /** @convert */ toLayoutRoot: () => LayoutRoot,
    /** @convert */ toNineSlice: () => NineSlice,
    /** @convert */ toSpineSprite: () => SpineSprite,
}

/**
 * @namespace krit
 * @import krit/sprites/BitmapText.h
 * @convertFrom VisibleSprite
 */
interface BitmapText extends VisibleSprite {
    /** @readonly */ text: string,
    refresh: () => void,
    setText: (s: string) => void,
    setRichText: (s: string) => void,
    baseScale: () => number,
    /** @readonly @getter width */ width: number,
    /** @readonly @getter height */ height: number,
}

/**
 * @namespace krit
 * @import krit/sprites/Image.h
 */
interface Image extends VisibleSprite {}

/**
 * @namespace krit
 * @import krit/sprites/NineSlice.h
 */
interface NineSlice extends VisibleSprite {}

/**
 * @namespace krit
 * @import krit/sprites/Backdrop.h
 */
interface Backdrop extends VisibleSprite {}

/**
 * @namespace krit
 * @import krit/sprites/SpineSprite.h
 */
interface SpineSprite extends VisibleSprite {}

/**
 * @namespace krit
 * @import krit/sprites/Layout.h
 * @pointerOnly
 */
interface LayoutNode extends VisibleSprite {
    x: AnchoredMeasurement,
    y: AnchoredMeasurement,
    width: Measurement,
    height: Measurement,
    spacing: Measurement,
    x: AnchoredMeasurement,
    y: AnchoredMeasurement,
    /** @readonly @getter paddingTop */ paddingTop: Measurement,
    /** @readonly @getter paddingBottom */ paddingBottom: Measurement,
    /** @readonly @getter paddingLeft */ paddingLeft: Measurement,
    /** @readonly @getter paddingRight */ paddingRight: Measurement,
    stretch: boolean,
    keepSize: boolean,
    visible: boolean,

    getSprite: () => Pointer<VisibleSprite>,
    attachSprite: (s: Pointer<VisibleSprite>) => void,
}

/**
 * @namespace krit
 * @import krit/sprites/Layout.h
 * @pointerOnly
 */
interface LayoutRoot extends Sprite {
    getById: (id: string) => Pointer<VisibleSprite>,
    getNodeById: (id: string) => Pointer<LayoutNode>,
}
