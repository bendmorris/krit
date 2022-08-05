// DataModel must be extended, and the child must be exposed separately.

interface DataModel<T> {
    get(index: size_t): Reference<T>;
    getById(id: string): Reference<T>;
    all(): Reference<Array<T>>;
    count(): number;
    reserve(n: size_t): void;
    newItem(): Reference<T>;
    finish(x: Reference<T>): void;
}
