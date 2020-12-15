/**
 * @namespace krit
 * @import krit/Sprite.h
 */
interface VisibleSprite {
    /** @readonly true */ position: Point,
    /** @readonly true */ dimensions: Dimensions,
    color: Color,
    render: (ctx: Reference<RenderContext>) => void,
}

/**
 * @namespace krit
 * @import krit/sprites/BitmapText.h
 */
interface BitmapText extends VisibleSprite {
    text: string,
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
