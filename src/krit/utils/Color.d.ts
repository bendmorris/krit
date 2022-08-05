declare class Color {
    r: number;
    g: number;
    b: number;
    a: number;

    constructor();
    constructor(partial: Partial<Color>);

    setTo(c: number): void;
    lerpInPlace(c: number, mix: number): void;
    rgb(): number;
}
