declare class Io {
    readFile(path: string): string;
    writeFile(path: string, contents: string): void;
    exists(path: string): boolean;
    isDirectory(path: string): boolean;
    rm(path: string): boolean;
    mkdir(path: string): boolean;
}
