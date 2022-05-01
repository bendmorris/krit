/**
 * @namespace krit
 * @import krit/math/Vec.h
 * @noForwardDeclare
 */
declare class Vec2f {
    /** @getter x */ x: float;
    /** @getter y */ y: float;

    constructor(x?: float, y?: float);

    copyFrom(other: Reference<Vec2f>): void;
    setTo(x: float, y: float): void;
    distance(p: Reference<Vec2f>): number;
    squaredDistance(p: Reference<Vec2f>): number;
}

/**
 * @namespace krit
 * @import krit/math/Vec.h
 * @noForwardDeclare
 */
declare class Vec3f {
    /** @getter x */ x: float;
    /** @getter y */ y: float;
    /** @getter z */ z: float;

    constructor(x?: float, y?: float, z?: float);

    copyFrom(other: Reference<Vec3f>): void;
    setTo(x: number, y: number, z?: number): void;
    distance(p: Reference<Vec3f>): number;
    squaredDistance(p: Reference<Vec3f>): number;
}

/**
 * @namespace krit
 * @import krit/math/Vec.h
 * @noForwardDeclare
 */
declare class Vec2i {
    /** @getter x */ x: integer;
    /** @getter y */ y: integer;

    constructor(x?: integer, y?: integer);

    copyFrom(other: Reference<Vec2i>): void;
    setTo(x: integer, y: integer): void;
    distance(p: Reference<Vec2i>): number;
    squaredDistance(p: Reference<Vec2i>): number;
}

/**
 * @namespace krit
 * @import krit/math/ScaleFactor.h
 */
declare class ScaleFactor extends Vec2f {
    setTo(x: float, y?: float): void;
}

/**
 * @import krit/math/Measurement.h
 */
declare enum MeasurementType {
    Absolute,
    Percent,
}

/**
 * @namespace krit
 * @import krit/math/Measurement.h
 */
interface Measurement {
    /** @cast MeasurementType */ type: integer;
    value: number;
    latest: number;
}

/**
 * @namespace krit
 * @import krit/math/Measurement.h
 */
interface AnchoredMeasurement {
    value: Measurement;
    anchor: number;
    latest: number;
}

/**
 * @namespace krit
 * @import krit/math/Rectangle.h
 */
declare class Rectangle {
    x: number;
    y: number;
    width: number;
    height: number;

    constructor(x: number, y: number, width: number, height: number);

    contains(x: number, y: number): boolean;
    copyFrom(other: Reference<Rectangle>): void;
    setTo(x: number, y: number, width: number, height: number): void;
}

/**
 * @namespace krit
 * @import krit/math/Rectangle.h
 */
declare class IntRectangle {
    x: integer;
    y: integer;
    width: integer;
    height: integer;

    constructor(x: integer, y: integer, width: integer, height: integer);

    contains(x: integer, y: integer): boolean;
    copyFrom(other: Reference<IntRectangle>): void;
    setTo(x: integer, y: integer, width: integer, height: integer): void;
}

/**
 * @namespace krit
 * @import krit/math/Triangle.h
 */
interface Triangle {
    p1: Vec3f;
    p2: Vec3f;
    p3: Vec3f;
}

type Point = Vec3f;
type IntPoint = Vec2i;
type Dimensions = Vec2f;
type IntDimensions = Vec2i;
