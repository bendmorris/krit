type Reference<T> = T & { __ref?: undefined }
type Pointer<T> = T & { __ptr?: undefined }
type StringMap<T> = Record<string, T>
type string_view = string & { __view?: undefined }
type integer = number;
type int64 = number;
type float = number;
type size_t = number;
/** */ declare function __id(x: any): int64;
/** */ declare function timeout(t: number): Promise<void>;
