declare class Rectangle {
    x: number;
    y: number;
    width: number;
    height: number;

    constructor(x: number, y: number, width: number, height: number);
    constructor(partial: Partial<Rectangle>);

    contains(p: Partial<Vec2f>): boolean;
    copyFrom(other: Rectangle): void;
    setTo(x: number, y: number, width: number, height: number): void;
}

declare class IntRectangle {
    x: number;
    y: number;
    width: number;
    height: number;

    constructor(x: number, y: number, width: number, height: number);
    constructor(partial: Partial<IntRectangle>);

    contains(p: Partial<Vec2i>): boolean;
    copyFrom(other: IntRectangle): void;
    setTo(x: number, y: number, width: number, height: number): void;
}
