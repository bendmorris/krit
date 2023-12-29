declare class NineSlice extends VisibleSprite {
    static from(value: Sprite): NineSlice;

    borderScale: Vec2f;

    constructor(region: ImageRegion, l: number, r: number, t: number, b: number);
}
