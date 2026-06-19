declare class Platform {
    readonly dataDir: string;
    readonly configDir: string;
    readonly name: string;
    joinPaths(path1: string, path2: string): string;

    exists(path: string): boolean;
    isDir(path: string): boolean;
    readFile(path: string): string;
    writeFile(path: string, content: string): void;
    copyFile(src: string, dest: string): boolean;
    moveFile(src: string, dest: string): boolean;
    createDir(path: string, /** @defaultValue false */ recursive?: boolean): boolean;
    readDir(path: string): Array<string>;
    remove(path: string, /** @defaultValue false */ recursive?: boolean): boolean;

    saveFileDialog(title: string, filters: string[]): string | undefined;
    openFileDialog(title: string, filters: string[]): string | undefined;

    getClipboardText(): string;
    setClipboardText(content: string): void;
}
