/** @skip */ declare const krit: Engine;

declare namespace console {
    /** @jsfunc */ function log(..._: any[]): void;
}

// declare function setTimeout(f: () => void, t: number): void;

/** @jsfunc */ declare function __id(x: any): number;
/** @jsfunc */ declare function timeout(t: number): Promise<void>;
/** @jsfunc */ declare function gc(): void;
/** @jsfunc */ declare function dumpMemoryUsage(): void;
/** @jsfunc */ declare function exit(): void;
/** @jsfunc */ declare function abort(msg: string): void;
/** @jsfunc */ declare function readFile(path: string): string;
/** @skip */ declare function __date_clock(): number;

declare namespace Log {
    /** @jsfunc */ function addLogSink(sink: (area: string, s: string, level: LogLevel) => void): void;
    /** @jsfunc */ function setLogLevel(level: LogLevel): void;
    /** @jsfunc */ function debug(msg: string): void;
    /** @jsfunc */ function info(msg: string): void;
    /** @jsfunc */ function warn(msg: string): void;
    /** @jsfunc */ function error(msg: string): void;
    /** @jsfunc */ function output(msg: string): void;
    /** @jsfunc */ function fatal(msg: string): void;
}

/** @jsfunc */ declare function encodeString(s: string): Uint8Array;
/** @jsfunc */ declare function decodeString(b: Uint8Array): string;
