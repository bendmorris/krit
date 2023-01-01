declare class FileIo {
    static configDir(): string;
    static dataDir(): string;
    static mkdir(path: string): void;
    static exists(path: string): boolean;
    static write(path: string, buf: string, size: number): void;
}
