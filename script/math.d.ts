/**
 * @namespace krit
 * @import krit/math/Point.h
 */
interface Point {
    x: number;
    y: number;

    setTo(x: number, y: number): void;
    distance(p: Reference<Point>): number;
    squaredDistance(p: Reference<Point>): number;
}

/**
 * @namespace krit
 * @import krit/math/Point.h
 */
interface IntPoint {
    x: integer;
    y: integer;

    setTo(x: integer, y: integer): void;
}

/**
 * @namespace krit
 * @import krit/math/Dimensions.h
 */
interface Dimensions {
    x: number;
    y: number;

    width(): Reference<number>;
    height(): Reference<number>;
    setTo(x: number, y: number): void;
}

/**
 * @namespace krit
 * @import krit/math/Dimensions.h
 */
interface IntDimensions {
    x: integer;
    y: integer;

    width(): Reference<integer>;
    height(): Reference<integer>;
}

/**
 * @namespace krit
 * @import krit/math/ScaleFactor.h
 */
interface ScaleFactor {
    x: number;
    y: number;

    setTo(x: number, y: number): void;
}

declare const enum MeasurementType {
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
interface Rectangle {
    x: number;
    y: number;
    width: number;
    height: number;

    contains(x: number, y: number): boolean;
}

/**
 * @namespace krit
 * @import krit/math/Rectangle.h
 */
 interface IntRectangle {
    x: integer;
    y: integer;
    width: integer;
    height: integer;

    contains(x: integer, y: integer): boolean;
}
