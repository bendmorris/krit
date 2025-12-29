declare class Sprite {
    fixedUpdate(): void;
    update(): void;
    render(ctx: RenderContext): void;
}

declare class VisibleSprite extends Sprite {
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
