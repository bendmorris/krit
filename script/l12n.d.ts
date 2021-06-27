/**
 * @namespace krit
 * @import krit/l12n/TextMap.h
 */
interface TextMap {
    /** @readonly */ locale: string;

    registerLocale(key: string, path: string): void;
    setLocale(key: string): void;
    getString(key: string): string_view;
}
