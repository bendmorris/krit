declare class DrawContext {
    blend: BlendMode;
    smooth: SmoothingMode;
    color: Color;
    alpha: number;
    lineThickness: number;
    zIndex: number;

    constructor(ctx: RenderContext);

    line(p1: Vec2f, p2: Vec2f): void;
    rect(r: Rectangle): void;
    rectFilled(r: Rectangle): void;
    circle(center: Vec2f, radius: number, segments: number): void;
    ring(center: Vec2f, radius: number, segments: number): void;
    circleFilled(center: Partial<Vec2f>, radius: number, segments: number): void;
    arc(center: Vec2f, radius: number, startRads: number, angleRads: number, segments: number): void;
    drawTriangle(p1: Vec2f, p2: Vec2f, p3: Vec2f): void;
}
