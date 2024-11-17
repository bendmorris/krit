declare class DrawContext {
    blend: BlendMode;
    smooth: SmoothingMode;
    shader: SpriteShader;
    color: Color;
    alpha: number;
    lineThickness: number;
    zIndex: number;

    constructor();

    line(p1: Partial<Vec2f>, p2: Partial<Vec2f>): void;
    rect(r: Rectangle): void;
    rectFilled(r: Rectangle): void;
    circle(center: Partial<Vec2f>, radius: number, segments: number): void;
    ring(center: Partial<Vec2f>, radius: number, segments: number): void;
    circleFilled(center: Partial<Vec2f>, radius: number, segments: number): void;
    arc(center: Partial<Vec2f>, radius: number, startRads: number, angleRads: number, segments: number): void;
    drawTriangle(p1: Partial<Vec2f>, p2: Partial<Vec2f>, p3: Partial<Vec2f>): void;
}
