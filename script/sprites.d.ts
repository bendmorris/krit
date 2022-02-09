/**
 * @import krit/render/BlendMode.h
 */
declare enum BlendMode {
    Alpha,
    Add,
    Subtract,
    Multiply,
    BlendScreen,
}

/**
 * @import krit/render/SmoothingMode.h
 */
declare enum SmoothingMode {
    SmoothNearest,
    SmoothLinear,
    SmoothMipmap,
}

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
    zIndex: integer;
    /** @readonly */ color: Color;
    /** @cast BlendMode */ blendMode: BlendMode;
    /** @cast SmoothingMode */ smooth: SmoothingMode;
    getBounds(): Rectangle;
    shader: Pointer<SpriteShader>;
}

/**
 * @namespace krit
 * @import krit/sprites/Text.h
 */
declare enum AlignType {
    LeftAlign,
    CenterAlign,
    RightAlign,
}

/**
 * @namespace krit
 * @import krit/sprites/Text.h
 */
declare class TextOptions {
    setFont(font: string): void;
    setSize(size: integer): void;
    setWordWrap(wrap: boolean): void;
    setAlign(/** @cast AlignType */ align: AlignType): void;

    constructor();
}

/**
 * @namespace krit
 * @import krit/sprites/Text.h
 */
declare class GlyphRenderData {
    c: integer;
    color: Color;
    scale: ScaleFactor;
    position: Point;
}

/**
 * @namespace krit
 * @import krit/sprites/Text.h
 */
declare class TextFormatTagOptions {
    constructor();
    setColor(c: integer): void;
    // setAlign(/* @cast AlignType */ a: AlignType): void;
    setNewline(): void;
    setTab(): void;
    // setCustom(f: (ctx: Pointer<RenderContext>, txt: Pointer<Text>, glyph: Pointer<GlyphRenderData>) => void): void;
    setSprite(s: Pointer<VisibleSprite>): void;
    setDelay(delay: integer): void;
    setBorder(): void;
}

/**
 * @namespace krit
 * @import krit/sprites/Text.h
 */
declare class Text extends VisibleSprite {
    static from(value: Sprite): Text;
    static addFormatTag(name: string, options: TextFormatTagOptions): void;

    constructor(options: Reference<TextOptions>);

    /** @readonly @getter width */ width: number;
    /** @readonly @getter height */ height: number;
    charCount: integer;
    /** @readonly */ maxChars: integer;
    /** @readonly */ text: string;
    baseColor: Color;
    /** @readonly */ textDimensions: Dimensions;
    allowPixelPerfect: boolean;
    border: boolean;
    borderThickness: integer;
    borderColor: Color;
    glyphScale: float;

    refresh(): void;
    setText(s: string): void;
    setRichText(s: string): void;
    setTabStops(s: string): void;
    setFontSize(s: integer): void;
}

/**
 * @namespace krit
 * @import krit/sprites/Image.h
 */
declare class Image extends VisibleSprite {
    static from(value: Sprite): Image;

    constructor(region: Reference<ImageRegion>);

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

    constructor(region: Reference<ImageRegion>, l: integer, r: integer, t: integer, b: integer);
}

/**
 * @namespace krit
 * @import krit/sprites/Backdrop.h
 */
declare class Backdrop extends VisibleSprite {
    static from(value: Sprite): Backdrop;

    constructor(region: Reference<ImageRegion>);
}

/**
 * @namespace krit
 * @import krit/sprites/SpineSprite.h
 * @pointerOnly
 */
declare class SpineSprite extends VisibleSprite {
    static from(value: Sprite): SpineSprite;
    static setAtlasPath(path: string): void;

    rate: number;

    setSkin(name: string): void;
    setAnimation(track: integer, name: string, loop: boolean, speed: number, mix: number): float;
    addAnimation(track: integer, name: string, loop: boolean, delay: number, mix: number): float;
    getAnimation(track: integer): cstring;
    advance(t: number): void;
}

/**
 * @namespace krit
 * @import krit/sprites/Emitter.h
 */
declare class Emitter extends VisibleSprite {
    static from(value: Sprite): Emitter;

    emit(name: string, count: number): void;
}
