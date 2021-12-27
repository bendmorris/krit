/**
 * @namespace krit
 * @import krit/l12n/TextMap.h
 */
declare class TextMap {
    /** @readonly */ locale: string;

    constructor();

    registerLocale(key: string, path: string): void;
    setLocale(key: string): void;
    getString(key: string): string_view;
}
