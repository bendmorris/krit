/**
 * @namespace
 */
declare const console: {
    log(..._: any[]): void;
};

// declare function setTimeout(f: () => void, t: number): void;

/** */ declare function __id(x: any): int64;
/** */ declare function timeout(t: number): Promise<void>;
/** */ declare function gc(): void;
/** */ declare function dumpMemoryUsage(): void;
/** */ declare function exit(): void;
/** */ declare function readFile(path: string): string;

declare function __date_clock(): number;

/**
 * @namespace krit
 * @import krit/io/FileIo.h
 */
declare class FileIo {
    static configDir(): string;
    static dataDir(): string;
    static mkdir(path: string): void;
    static exists(path: string): boolean;
    static write(path: string, buf: string): void;
}

/**
 * @namespace krit
 * @import krit/io/FileIo.h
 */
declare class Log {
    static debug(s: cstring): void;
    static info(s: cstring): void;
    static warn(s: cstring): void;
    static error(s: cstring): void;
    static fatal(s: cstring): void;
    static success(s: cstring): void;
}
