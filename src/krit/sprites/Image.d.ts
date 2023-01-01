declare class Image extends VisibleSprite {
    static from(value: Sprite): Image;

    constructor(region: ImageRegion);

    origin: Vec3f;
    get width(): number;
    get height(): number;
    angle: number;
    pitch: number;
    region: ImageRegion;
    centerOrigin(): void;
}
