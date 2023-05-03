// FIXME: rename, since this interface is read only and intended for assets
declare class Io {
    addRoot(path: string): boolean;
    hasRoot(path: string): boolean;
    enableRoot(path: string): boolean;
    disableRoot(path: string): boolean;

    exists(path: string): boolean;
    readFile(path: string): string;
    readDir(path: string): Array<string>;
}
