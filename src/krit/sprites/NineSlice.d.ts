declare class NineSlice extends VisibleSprite {
    static from(value: Sprite): NineSlice;

    constructor(region: ImageRegion, l: number, r: number, t: number, b: number);
}
