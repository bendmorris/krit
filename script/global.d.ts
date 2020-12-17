type Reference<T> = T & { __ref?: undefined }
type Pointer<T> = T & { __ptr?: undefined }
type StringMap<T> = Record<string, T>
interface integer extends Number {}
interface float extends Number {}
