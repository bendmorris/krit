declare class Sprite {
    position: Vec3f;
    dimensions: Vec2f;
    zIndex: number;
    color: Color;
    blendMode: BlendMode;
    smooth: SmoothingMode;
    shader: SpriteShader;
    fixedUpdate(): void;
    update(): void;
    render(ctx: Ref<RenderContext>): void;
    get width(): number;
    set width(x: number);
    get height(): number;
    set height(x: number);
}
