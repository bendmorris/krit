declare class DbRow {
    getInt(index: number): number;
    getString(index: number): string;
    getBlob(index: number): ArrayBuffer;
}

declare class FileDb {
    constructor(path: string);
    valid: boolean;
    exec(query: string, callback?: (row: object) => void): void;
    prepare(query: string): UniquePtr<DbQuery>;
}

declare class DbQuery {
    readonly columnNames: string[];
    readonly columnCount: number;
    bindInt(index: number, val: number): void;
    bindString(index: number, val: string): void;
    bindBlob(index: number, val: ArrayBuffer): void;
    exec(callback?: (row: object) => void): void;
}
