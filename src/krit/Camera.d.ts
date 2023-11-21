declare class Camera {
    position: Vec3f;
    offset: Vec2i;
    anchor: Vec3f;
    scale: Vec2f;
    dimensions: Vec2f;
    currentDimensions: Vec2f;
    rotation: number;
    pitch: number;
    roll: number;
    minRatio: number;
    maxRatio: number;

    center(): void;
    update(ctx: RenderContext): void;
    transformPoint(p: Vec3f): void;
    untransformPoint(p: Vec3f): void;
    resetRotation(): void;
    setLogicalSize(w: number, h: number): Camera;
    viewportWidth(): number;
    viewportHeight(): number;

    screenToWorldCoords(p: Vec3f): void;
}
