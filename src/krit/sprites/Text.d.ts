declare enum AlignType {
    LeftAlign,
    CenterAlign,
    RightAlign,
}

declare class TextOptions {
    setFont(font: string): void;
    setSize(size: number): void;
    setWordWrap(wrap: boolean): void;
    setAlign(align: AlignType): void;

    constructor();
}

declare class GlyphRenderData {
    c: number;
    color: Color;
    scale: Vec2f;
    position: Vec3f;
}

declare class TextFormatTagOptions {
    constructor();
    setColor(c: number): void;
    setNewline(): void;
    setTab(): void;
    setSprite(s: VisibleSprite): void;
    setDelay(delay: number): void;
    setBorder(): void;
}

declare class Text extends VisibleSprite {
    static from(value: Sprite): Text;
    static addFormatTag(name: string, options: TextFormatTagOptions): void;

    constructor(options: TextOptions);

    get width(): number;
    get height(): number;
    charCount: number;
    readonly maxChars: number;
    readonly text: string;
    readonly textDimensions: Vec2f;
    baseColor: Color;
    allowPixelPerfect: boolean;
    dynamicSize: boolean;
    border: boolean;
    borderThickness: number;
    borderColor: Color;
    glyphScale: number;

    refresh(): void;
    invalidate(): void;
    setText(s: string): void;
    setRichText(s: string): void;
    setTabStops(s: string): void;
    setFontSize(s: number): void;
}
