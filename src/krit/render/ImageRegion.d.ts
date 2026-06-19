declare class ImageRegion {
    rect: IntRectangle;
    constructor();
    constructor(img: SharedPtr<ImageData>);
    constructor(img: SharedPtr<ImageData>, x: number, y: number, width: number, height: number);
}
