interface Window extends Vec2i {
    isFullScreen(): boolean;
    setFullScreen(full: boolean): void;
    setWindowSize(width: number, height: number): void;
}
