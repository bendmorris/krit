declare enum MouseButton {
    MouseLeft,
    MouseMiddle,
    MouseRight,
    MouseWheel,
    MouseButtonMax,
}

interface MouseContext {
    mousePos: Vec3i;
}
