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

declare function __date_clock(): number;
