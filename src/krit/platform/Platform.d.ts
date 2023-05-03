declare class Platform {
    get dataDir(): string;
    get configDir(): string;
    get name(): string;
    joinPaths(path1: string, path2: string): string;

    exists(path: string): boolean;
    isDir(path: string): boolean;
    readFile(path: string): string;
    writeFile(path: string, content: string): void;
    copyFile(src: string, dest: string): boolean;
    moveFile(src: string, dest: string): boolean;
    createDir(path: string, recursive?: boolean): boolean;
    readDir(path: string): Array<string>;
    remove(path: string, recursive?: boolean): boolean;
}
