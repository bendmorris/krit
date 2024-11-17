declare class InputContext {
    readonly events: ActionEvent[];
    mouse: MouseContext;
    key: KeyContext;
    state(a: number): number;

    bindKey(key: KeyCode, action: number): void;
    bindMouse(button: MouseButton, action: number): void;
}

interface ActionEvent {
    action: number;
    state: number;
    prevState: number;
}
