declare class TextMap {
    readonly locale: string;

    constructor();

    registerLocale(key: string, path: string): void;
    setLocale(key: string): void;
    getString(key: string): string;
}
