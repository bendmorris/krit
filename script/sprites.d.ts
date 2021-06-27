/**
 * @namespace krit
 * @import krit/Sprite.h
 */
declare class Sprite {
    fixedUpdate(ctx: Reference<UpdateContext>): void;
    update(ctx: Reference<UpdateContext>): void;
    render(ctx: Reference<RenderContext>): void;
}

/**
 * @namespace krit
 * @import krit/Sprite.h
 */
declare class VisibleSprite extends Sprite {
    static from(value: Sprite): VisibleSprite;

    /** @readonly */ position: Point;
    /** @readonly */ dimensions: Dimensions;
    /** @readonly */ scale: ScaleFactor;
    color: Color;
}

/**
 * @namespace krit
 * @import krit/sprites/BitmapText.h
 */
declare class BitmapText extends VisibleSprite {
    static from(value: Sprite): BitmapText;

    /** @readonly @getter width */ width: number;
    /** @readonly @getter height */ height: number;
    charCount: integer;
    /** @readonly */ maxChars: integer;
    /** @readonly */ text: string;
    baseColor: Color;

    refresh(): void;
    setText(s: string): void;
    setRichText(s: string): void;
    baseScale(): number;
}

/**
 * @namespace krit
 * @import krit/sprites/Text.h
 */
 declare class Text extends VisibleSprite {
    static from(value: Sprite): Text;

    /** @readonly @getter width */ width: number;
    /** @readonly @getter height */ height: number;
    charCount: integer;
    /** @readonly */ maxChars: integer;
    /** @readonly */ text: string;
    baseColor: Color;
    /** @readonly */ textDimensions: Dimensions;

    refresh(): void;
    setText(s: string): void;
    setRichText(s: string): void;
}

/**
 * @namespace krit
 * @import krit/sprites/Image.h
 */
declare class Image extends VisibleSprite {
    static from(value: Sprite): Image;

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
declare class NineSlice extends VisibleSprite {
    static from(value: Sprite): NineSlice;
}

/**
 * @namespace krit
 * @import krit/sprites/Backdrop.h
 */
declare class Backdrop extends VisibleSprite {
    static from(value: Sprite): Backdrop;
}

/**
 * @namespace krit
 * @import krit/sprites/SpineSprite.h
 */
declare class SpineSprite extends VisibleSprite {
    static from(value: Sprite): SpineSprite;

    setSkin(name: string): void;
    setAnimation(track: integer, name: string, loop: boolean): float;
}

/**
 * @namespace krit
 * @import krit/sprites/Layout.h
 * @pointerOnly
 */
declare class LayoutNode extends VisibleSprite {
    static from(value: Sprite): LayoutNode;

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
    /** @readonly */ offset: Point;

    getSprite(): Pointer<VisibleSprite>;
    attachSprite(s: Pointer<VisibleSprite>): void;
    addChild(node: Pointer<LayoutNode>): void;
    clearChildren(): void;
}

/**
 * @namespace krit
 * @import krit/sprites/Layout.h
 * @pointerOnly
 */
declare class LayoutRoot extends Sprite {
    static from(value: Sprite): LayoutRoot;

    getById(id: string): Pointer<VisibleSprite>;
    getNodeById(id: string): Pointer<LayoutNode>;
}
