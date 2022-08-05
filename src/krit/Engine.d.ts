interface Engine {
    input: InputContext;
    speed: number;
    elapsed: number;
    bgColor: Color;

    cameras: Array<Camera>;
    setCursor(c: string): void;

    getImage(id: string): ImageData;
    getAtlas(id: string): TextureAtlas;
}
