/**
 * @namespace krit
 * @import krit/data/DataModel.h
 */
interface DataItem {
    id: string;
}

// DataModel must be extended, and the child must be exposed separately.

interface DataModel<T> {
    getById(id: string): Reference<T>;
    newItem(): Reference<T>;
    finish(x: Reference<T>): void;
}
