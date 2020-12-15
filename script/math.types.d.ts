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
interface Dimensions extends Point {}
