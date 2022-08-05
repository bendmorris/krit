declare class ParticleSystem extends VisibleSprite {
    static from(value: Sprite): ParticleSystem;
    static clone(value: ParticleSystem): ParticleSystem;
    constructor();
    loadAtlas(path: string): void;
    loadEffect(path: string): void;
    emit(name: string, p: Partial<Vec3f>, loop?: boolean): EffectInstance;
    clear(): void;
    hasParticles(): boolean;
}

declare class EffectInstance {
    origin: Vec3f;
    angle: number;
}
