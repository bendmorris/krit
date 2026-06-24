declare class Vec2f {
    constructor(partial: Partial<Vec2f>);
    constructor(/** @defaultValue 0 */ x?: float, /** @defaultValue 0 */ y?: float);

    x: float;
    y: float;

    clone(): Vec2f;
    copyFrom(other: Partial<Vec2f>): void;
    setTo(x: float, y: float): void;
    distance(p: Partial<Vec2f>): float;
    squaredDistance(p: Partial<Vec2f>): float;
    normalize(): void;
}

declare class Vec3f {
    constructor(partial: Partial<Vec3f>);
    constructor(/** @defaultValue 0 */ x?: float, /** @defaultValue 0 */ y?: float, /** @defaultValue 0 */ z?: float);

    x: float;
    y: float;
    z: float;

    clone(): Vec3f;
    copyFrom(other: Partial<Vec3f>): void;
    setTo(x: float, y: float, /** @defaultValue 0 */ z?: float): void;
    distance(p: Partial<Vec3f>): float;
    squaredDistance(p: Partial<Vec3f>): float;
    normalize(): void;
}

declare class Vec2i {
    constructor(partial: Partial<Vec2i>);
    constructor(/** @defaultValue 0 */ x?: int, /** @defaultValue 0 */ y?: int);

    x: int;
    y: int;

    clone(): Vec2i;
    copyFrom(other: Partial<Vec2i>): void;
    setTo(x: int, y: int): void;
    distance(p: Partial<Vec2i>): float;
    squaredDistance(p: Partial<Vec2i>): float;
    normalize(): void;
}

declare class Vec3i {
    constructor(partial: Partial<Vec3i>);
    constructor(/** @defaultValue 0 */ x?: int, /** @defaultValue 0 */ y?: int, /** @defaultValue 0 */ z?: int);

    x: int;
    y: int;
    z: int;

    clone(): Vec2i;
    copyFrom(other: Partial<Vec3i>): void;
    setTo(x: int, y: int, z: int): void;
    distance(p: Partial<Vec3i>): int;
    squaredDistance(p: Partial<Vec3i>): int;
    normalize(): void;
}
