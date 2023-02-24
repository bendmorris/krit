declare class DbRow {
    getInt(index: number): number;
    getString(index: number): string;
    getBlob(index: number): ArrayBuffer;
}

declare class FileDb {
    constructor(path: string);
    valid: boolean;
    exec(query: string, callback?: (row: DbRow) => void): void;
    prepare(query: string): DbQuery;
}

declare class DbQuery {
    get columnNames(): string[];
    get columnCount(): number;
    bindInt(index: number, val: number): void;
    bindString(index: number, val: string): void;
    bindBlob(index: number, val: ArrayBuffer): void;
    exec(callback?: (row: object) => void): void;
}
