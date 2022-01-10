/**
 * @namespace krit
 * @import krit/math/Point.h
 */
declare class Point {
    x: number;
    y: number;

    constructor(x: number, y: number);

    setTo(x: number, y: number): void;
    distance(p: Reference<Point>): number;
    squaredDistance(p: Reference<Point>): number;
}

/**
 * @namespace krit
 * @import krit/math/Point.h
 */
declare class IntPoint {
    x: integer;
    y: integer;

    constructor(x: integer, y: integer);

    setTo(x: integer, y: integer): void;
}

/**
 * @namespace krit
 * @import krit/math/Dimensions.h
 */
declare class Dimensions {
    x: number;
    y: number;

    constructor(x: number, y: number);

    width(): Reference<number>;
    height(): Reference<number>;
    setTo(x: number, y: number): void;
}

/**
 * @namespace krit
 * @import krit/math/Dimensions.h
 */
declare class IntDimensions {
    x: integer;
    y: integer;

    constructor(x: integer, y: integer);

    width(): Reference<integer>;
    height(): Reference<integer>;
}

/**
 * @namespace krit
 * @import krit/math/ScaleFactor.h
 */
declare class ScaleFactor {
    x: number;
    y: number;

    constructor(x: number, y: number);

    setTo(x: number, y: number): void;
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
    setTo(x: integer, y: integer, width: integer, height: integer): void;
}

/**
 * @namespace krit
 * @import krit/math/Triangle.h
 */
interface Triangle {
    p1: Point;
    p2: Point;
    p3: Point;
}
