declare class Backdrop extends VisibleSprite {
    static from(value: Sprite): Backdrop;

    pitch: number;

    constructor(id: ImageRegion);
}
