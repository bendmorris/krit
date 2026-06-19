declare class Image extends Sprite {
    constructor();
    constructor(path: string);
    constructor(img: ImageRegion);

    origin: Vec3f;
    angle: number;
    pitch: number;
    region: ImageRegion;
    centerOrigin(): void;
    setScale(scaleX: number, scaleY?: number): void;

    set src(img: ImageRegion);
    set scale(s: number);
    get scaleX(): number;
    set scaleX(sx: number);
    get scaleY(): number;
    set scaleY(sy: number);
}
