/**
 * @namespace krit
 * @import krit/utils/Color.h
 */
declare class Color {
    r: float;
    g: float;
    b: float;
    a: float;

    constructor();

    setTo(c: integer): void;
    lerpInPlace(c: integer, mix: number): void;
}

/**
 * @import krit/utils/Log.h
 */
declare enum LogLevel {
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    Success,
}

/** @namespace */
declare const Log: {
    setLogLevel(level: LogLevel): void;
    debug(msg: string): void;
    info(msg: string): void;
    warn(msg: string): void;
    error(msg: string): void;
    fatal(msg: string): void;
    success(msg: string): void;
}
