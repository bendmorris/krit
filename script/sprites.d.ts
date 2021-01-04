/**
 * @namespace krit
 * @import krit/Sprite.h
 */
interface Sprite {
    update(ctx: Reference<UpdateContext>): void;
    render(ctx: Reference<RenderContext>): void;

    /** @convert */ toVisibleSprite(): VisibleSprite;
    /** @convert */ toBackdrop(): Backdrop;
    /** @convert */ toBitmapText(): BitmapText;
    /** @convert */ toImage(): Image;
    /** @convert */ toLayoutNode(): LayoutNode;
    /** @convert */ toLayoutRoot(): LayoutRoot;
    /** @convert */ toNineSlice(): NineSlice;
    /** @convert */ toSpineSprite(): SpineSprite;
}

/**
 * @namespace krit
 * @import krit/Sprite.h
 */
interface VisibleSprite extends Sprite {
    /** @readonly */ position: Point;
    /** @readonly */ dimensions: Dimensions;
    /** @readonly */ scale: ScaleFactor;
    color: Color;
}

/**
 * @namespace krit
 * @import krit/sprites/BitmapText.h
 * @convertFrom VisibleSprite
 */
interface BitmapText extends VisibleSprite {
    /** @readonly @getter width */ width: number;
    /** @readonly @getter height */ height: number;
    charCount: integer;
    /** @readonly */ maxChars: integer;
    /** @readonly */ text: string;

    refresh(): void;
    setText(s: string): void;
    setRichText(s: string): void;
    baseScale(): number;
}

/**
 * @namespace krit
 * @import krit/sprites/Image.h
 */
interface Image extends VisibleSprite {
    /** @readonly */ origin: Point;
    /** @readonly @getter width */ width: integer;
    /** @readonly @getter height */ height: integer;
    angle: float;
    centerOrigin(): void;
}

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
interface SpineSprite extends VisibleSprite {
    setSkin(name: string): void;
    setAnimation(track: integer, name: string, loop: boolean): float;
}

/**
 * @namespace krit
 * @import krit/sprites/Layout.h
 * @pointerOnly
 */
interface LayoutNode extends VisibleSprite {
    x: AnchoredMeasurement;
    y: AnchoredMeasurement;
    width: Measurement;
    height: Measurement;
    spacing: Measurement;
    /** @readonly @getter paddingTop */ paddingTop: Measurement;
    /** @readonly @getter paddingBottom */ paddingBottom: Measurement;
    /** @readonly @getter paddingLeft */ paddingLeft: Measurement;
    /** @readonly @getter paddingRight */ paddingRight: Measurement;
    stretch: boolean;
    keepSize: boolean;
    visible: boolean;

    getSprite(): Pointer<VisibleSprite>;
    attachSprite(s: Pointer<VisibleSprite>): void;
}

/**
 * @namespace krit
 * @import krit/sprites/Layout.h
 * @pointerOnly
 */
interface LayoutRoot extends Sprite {
    getById(id: string): Pointer<VisibleSprite>;
    getNodeById(id: string): Pointer<LayoutNode>;
}
