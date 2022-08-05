declare class DataItem {
    id: string;
}

/** @skip */ interface DataModel<T> {
    get(index: number): T;
    getById(id: string): T;
    all(): Array<T>;
    count(): number;
    reserve(n: number): void;
    newItem(): T;
    finish(x: T): void;
}
