declare class InputContext {
    readonly events: ActionEvent[];
    mouse: MouseContext;
    key: KeyContext;
    state(a: number): number;

    bindKey(key: Key, action: number): void;
    bindMouse(button: MouseButton, action: number): void;
}

interface KeyContext {
    define(key: Key, action: number): void;
    undefine(key: Key): void;
    keyName(key: Key): string;
}

interface MouseContext {
    mousePos: Vec3i;
}

interface ActionEvent {
    action: number;
    state: number;
    prevState: number;
}
