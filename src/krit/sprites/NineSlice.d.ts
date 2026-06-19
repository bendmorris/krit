declare class NineSlice extends Sprite {
    borderScale: number;
    leftWidth: number;
    rightWidth: number;
    topHeight: number;
    bottomHeight: number;
    set borderWidth(v: number);
    set borderHeight(v: number);
    set border(v: number);

    constructor();
    constructor(path: string, borderX: number, borderY: number);
    constructor(path: string, borderLeft: number, borderRight: number, borderTop: number, borderBottom: number);
    constructor(path: ImageRegion, borderX: number, borderY: number);
    constructor(path: ImageRegion, borderLeft: number, borderRight: number, borderTop: number, borderBottom: number);

    set src(img: ImageRegion);
}
