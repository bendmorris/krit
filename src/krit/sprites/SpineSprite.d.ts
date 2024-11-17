declare class SpineSprite extends VisibleSprite {
    static from(value: Sprite): SpineSprite;
    static setAtlasPath(path: string): void;

    origin: Vec2f;
    rate: number;
    angle: number;
    pitch: number;

    constructor(path: string);

    setSkin(name: string): void;
    setAnimation(track: number, name: string, loop: boolean, speed: number, mix: number): number;
    addAnimation(track: number, name: string, loop: boolean, delay: number, mix: number): number;
    getAnimation(track: number): string;
    hasAnimation(name: string): boolean;
    stopAnimation(track: number): void;
    animationNames(): string[];
    advance(t: number): void;
}
