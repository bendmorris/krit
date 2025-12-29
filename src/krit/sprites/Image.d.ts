declare class Image extends VisibleSprite {
    constructor(region: ImageRegion);

    origin: Vec3f;
    get width(): number;
    get height(): number;
    angle: number;
    pitch: number;
    region: ImageRegion;
    centerOrigin(): void;
}
