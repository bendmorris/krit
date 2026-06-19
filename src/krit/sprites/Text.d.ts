declare enum AlignType {
    LeftAlign,
    CenterAlign,
    RightAlign,
}

declare class TextOptions {
    constructor();
    setFont(font: string): void;
    setSize(size: number): void;
    setWordWrap(wrap: boolean): void;
    setAlign(align: AlignType): void;
}

declare class GlyphRenderData {
    c: number;
    color: Color;
    scale: Vec2f;
    position: Vec3f;
}

declare class TextFormatTagOptions {
    constructor();
    setFont(f: string): void;
    setColor(c: number): void;
    setNewline(): void;
    setTab(): void;
    setSprite(s: Ptr<Sprite>): void;
    setDelay(delay: number): void;
    setBorder(): void;
}

declare class Text extends Sprite {
    static addFormatTag(name: string, options: TextFormatTagOptions): void;

    constructor(options: TextOptions);
    constructor();

    readonly content: string;
    readonly maxChars: number;
    readonly textDimensions: Vec2f;
    charCount: number;
    baseColor: Color;
    allowPixelPerfect: boolean;
    dynamicSize: boolean;
    border: boolean;
    borderThickness: number;
    borderColor: Color;
    glyphScale: number;
    pitch: number;
    scale: Vec2f;
    wordWrap: boolean;

    refresh(): void;
    invalidate(): void;
    set font(s: string);
    set text(s: string);
    set richText(s: string);
    set tabStops(s: string);
    set fontSize(s: number);
    set align(align: AlignType);
}
