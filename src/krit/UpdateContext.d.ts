/** skip */ declare const frame: UpdateContext;

declare class UpdateContext {
    readonly elapsed: number;
    readonly frameId: number;
    readonly tickId: number;
}
