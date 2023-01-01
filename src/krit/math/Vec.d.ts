declare class Vec2f {
    constructor(x?: number, y?: number);
    constructor(partial: Partial<Vec2f>);

    get x(): number;
    set x(x: number);
    get y(): number;
    set y(y: number);

    copyFrom(other: Partial<Vec2f>): void;
    setTo(x: number, y: number): void;
    distance(p: Partial<Vec2f>): number;
    squaredDistance(p: Partial<Vec2f>): number;
    normalize(): void;
}

declare class Vec3f {
    constructor(x?: number, y?: number, z?: number);
    constructor(partial: Partial<Vec3f>);

    get x(): number;
    set x(x: number);
    get y(): number;
    set y(y: number);
    get z(): number;
    set z(z: number);

    copyFrom(other: Partial<Vec3f>): void;
    setTo(x: number, y: number, z?: number): void;
    distance(p: Partial<Vec3f>): number;
    squaredDistance(p: Partial<Vec3f>): number;
    normalize(): void;
}

declare class Vec2i {
    constructor(x?: number, y?: number);
    constructor(partial: Partial<Vec2i>);

    get x(): number;
    set x(x: number);
    get y(): number;
    set y(y: number);

    copyFrom(other: Partial<Vec2i>): void;
    setTo(x: number, y: number): void;
    distance(p: Partial<Vec2i>): number;
    squaredDistance(p: Partial<Vec2i>): number;
    normalize(): void;
}

declare class Vec3i {
    constructor(x?: number, y?: number, z?: number);
    constructor(partial: Partial<Vec3i>);

    get x(): number;
    set x(x: number);
    get y(): number;
    set y(y: number);
    get z(): number;
    set z(z: number);

    copyFrom(other: Partial<Vec3i>): void;
    setTo(x: number, y: number): void;
    distance(p: Partial<Vec3i>): number;
    squaredDistance(p: Partial<Vec3i>): number;
    normalize(): void;
}
