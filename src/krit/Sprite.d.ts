declare class Sprite {
    fixedUpdate(ctx: UpdateContext): void;
    update(ctx: UpdateContext): void;
    render(ctx: RenderContext): void;
}

declare class VisibleSprite extends Sprite {
    static from(value: Sprite): VisibleSprite;

    position: Vec3f;
    dimensions: Vec2f;
    scale: Vec2f;
    zIndex: number;
    color: Color;
    blendMode: BlendMode;
    smooth: SmoothingMode;
    getBounds(): Rectangle;
    shader: SpriteShader;
}
