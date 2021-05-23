/**
 * @namespace krit
 * @import krit/utils/Color.h
 */
interface Color {
    r: float,
    g: float,
    b: float,
    a: float,

    setTo(c: integer): void;
    lerpInPlace(c: integer, mix: number): void;
}
