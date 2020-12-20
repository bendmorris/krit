/**
 * @namespace krit
 * @import krit/math/Point.h
 */
interface Point {
    x: number,
    y: number,

    setTo: (x: number, y: number) => void,
}

/**
 * @namespace krit
 * @import krit/math/Point.h
 */
interface IntPoint {
    x: integer,
    y: integer,

    setTo: (x: integer, y: integer) => void,
}

/**
 * @namespace krit
 * @import krit/math/Dimensions.h
 */
interface Dimensions extends Point {
    width: () => Reference<number>,
    height: () => Reference<number>,
}

/**
 * @namespace krit
 * @import krit/math/Dimensions.h
 */
interface IntDimensions extends IntPoint {
    width: () => Reference<integer>,
    height: () => Reference<integer>,
}

/**
 * @namespace krit
 * @import krit/math/ScaleFactor.h
 */
interface ScaleFactor extends Point {}

const enum MeasurementType {
    Absolute,
    Percent,
}

/**
 * @namespace krit
 * @import krit/math/Measurement.h
 */
interface Measurement {
    /** @cast MeasurementType */ type: integer,
    value: number,
}

/**
 * @namespace krit
 * @import krit/math/Measurement.h
 */
interface AnchoredMeasurement {
    value: Measurement,
    anchor: number,
}
